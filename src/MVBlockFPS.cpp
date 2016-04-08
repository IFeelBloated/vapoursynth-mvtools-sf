#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "VapourSynth.h"
#include "VSHelper.h"
#include "CopyCode.h"
#include "CommonFunctions.h"
#include "MaskFun.h"
#include "MVFilter.h"
#include "SimpleResize.h"

struct MVBlockFPSData {
	VSNodeRef *node;
	VSVideoInfo vi;
	const VSVideoInfo *supervi;
	VSNodeRef *super;
	VSNodeRef *mvbw;
	VSNodeRef *mvfw;
	int64_t num, den;
	int32_t mode;
	int32_t thres;
	bool blend;
	double thscd1, thscd2;
	MVClipDicks *mvClipB;
	MVClipDicks *mvClipF;
	MVFilter *bleh;
	int32_t nSuperHPad;
	int32_t nSuperVPad;
	int32_t nSuperPel;
	int32_t nSuperModeYUV;
	int32_t nSuperLevels;
	int32_t nWidthUV;
	int32_t nHeightUV;
	int32_t nPitchY;
	int32_t nPitchUV;
	int32_t nWidthP;
	int32_t nHeightP;
	int32_t nWidthPUV;
	int32_t nHeightPUV;
	int32_t nBlkXP;
	int32_t nBlkYP;
	uint8_t *OnesBlock;
	SimpleResize *upsizer;
	SimpleResize *upsizerUV;
	int64_t fa, fb;
	COPYFunction BLITLUMA;
};

static void VS_CC mvblockfpsInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData *d = reinterpret_cast<MVBlockFPSData *>(*instanceData);
	vsapi->setVideoInfo(&d->vi, 1, node);
}

static void MakeSmallMask(uint8_t *image, int32_t imagePitch, uint8_t *smallmask, int32_t nBlkX, int32_t nBlkY, int32_t nBlkSizeX, int32_t nBlkSizeY, int32_t threshold) {
	uint8_t *psmallmask = smallmask;
	for (int32_t ny = 0; ny < nBlkY; ny++) {
		for (int32_t nx = 0; nx < nBlkX; nx++) {
			psmallmask[nx] = 0;
			for (int32_t j = 0; j < nBlkSizeY; j++) {
				for (int32_t i = 0; i < nBlkSizeX; i++)
					if (image[i] == 0)
						psmallmask[nx]++;
				image += imagePitch;
			}
			image += -imagePitch * nBlkSizeY + nBlkSizeX;
		}
		image += imagePitch * nBlkSizeY - nBlkX * nBlkSizeX;
		psmallmask += nBlkX;
	}
	psmallmask = smallmask;
	for (int32_t ny = 0; ny < nBlkY; ny++) {
		for (int32_t nx = 0; nx < nBlkX; nx++) {
			if (psmallmask[nx] >= threshold)
				psmallmask[nx] = 255;
			else
				psmallmask[nx] = 0;

		}
		psmallmask += nBlkX;
	}
}

static void InflateMask(uint8_t *smallmask, int32_t nBlkX, int32_t nBlkY) {
	uint8_t *psmallmask = smallmask + nBlkX;
	for (int32_t ny = 1; ny < nBlkY - 1; ny++) {
		for (int32_t nx = 1; nx < nBlkX - 1; nx++) {
			if (psmallmask[nx] == 255) {
				psmallmask[nx - 1] = 192;
				psmallmask[nx + 1] = 192;
				psmallmask[nx - nBlkX - 1] = 144;
				psmallmask[nx - nBlkX] = 192;
				psmallmask[nx - nBlkX + 1] = 144;
				psmallmask[nx + nBlkX - 1] = 144;
				psmallmask[nx + nBlkX] = 192;
				psmallmask[nx + nBlkX + 1] = 144;
			}
		}
		psmallmask += nBlkX;
	}
}

static void MultMasks(uint8_t *smallmaskF, uint8_t *smallmaskB, uint8_t *smallmaskO, int32_t nBlkX, int32_t nBlkY) {
	for (int32_t j = 0; j < nBlkY; j++) {
		for (int32_t i = 0; i < nBlkX; i++)
			smallmaskO[i] = (smallmaskF[i] * smallmaskB[i]) / 255;
		smallmaskF += nBlkX;
		smallmaskB += nBlkX;
		smallmaskO += nBlkX;
	}
}

template <typename PixelType>
static inline PixelType MEDIAN(PixelType a, PixelType b, PixelType c) {
	PixelType mn = VSMIN(a, b);
	PixelType mx = VSMAX(a, b);
	PixelType m = VSMIN(mx, c);
	m = VSMAX(mn, m);
	return m;
}

template <typename PixelType>
static void RealResultBlock(uint8_t *pDst, int32_t dst_pitch, const uint8_t * pMCB, int32_t MCB_pitch, const uint8_t * pMCF, int32_t MCF_pitch,
	const uint8_t * pRef, int32_t ref_pitch, const uint8_t * pSrc, int32_t src_pitch, uint8_t *maskB, int32_t mask_pitch, uint8_t *maskF,
	uint8_t *pOcc, int32_t nBlkSizeX, int32_t nBlkSizeY, int32_t time256, int32_t mode) {
	if (mode == 0) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				PixelType *pDst_ = (PixelType *)pDst;
				double mca = (static_cast<double>(pMCB_[w]) * time256 + pMCF_[w] * (256. - time256)) / 256.;
				pDst_[w] = static_cast<PixelType>(mca);
			}
			pDst += dst_pitch;
			pMCB += MCB_pitch;
			pMCF += MCF_pitch;
		}
	}
	else if (mode == 1) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				const PixelType *pRef_ = (const PixelType *)pRef;
				const PixelType *pSrc_ = (const PixelType *)pSrc;
				PixelType *pDst_ = (PixelType *)pDst;
				double mca = (static_cast<double>(pMCB_[w]) * time256 + pMCF_[w] * (256. - time256)) / 256.;
				PixelType sta = MEDIAN<PixelType>(pRef_[w], pSrc_[w], static_cast<PixelType>(mca));
				pDst_[w] = sta;
			}
			pDst += dst_pitch;
			pMCB += MCB_pitch;
			pMCF += MCF_pitch;
			pRef += ref_pitch;
			pSrc += src_pitch;
		}
	}
	else if (mode == 2) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				const PixelType *pRef_ = (const PixelType *)pRef;
				const PixelType *pSrc_ = (const PixelType *)pSrc;
				PixelType *pDst_ = (PixelType *)pDst;
				double avg = (static_cast<double>(pRef_[w]) * time256 + pSrc_[w] * (256. - time256)) / 256.;
				PixelType dyn = MEDIAN<PixelType>(static_cast<PixelType>(avg), pMCB_[w], pMCF_[w]);
				pDst_[w] = dyn;
			}
			pDst += dst_pitch;
			pMCB += MCB_pitch;
			pMCF += MCF_pitch;
			pRef += ref_pitch;
			pSrc += src_pitch;
		}
	}
	else if (mode == 3) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				PixelType *pDst_ = (PixelType *)pDst;
				pDst_[w] = (((maskB[w] * pMCF_[w] + (255 - maskB[w]) * pMCB_[w] + 255) / 256) * time256 +
					((maskF[w] * pMCB_[w] + (255 - maskF[w]) * pMCF_[w] + 255) / 256) * (256 - time256)) / 256;
			}
			pDst += dst_pitch;
			pMCB += MCB_pitch;
			pMCF += MCF_pitch;
			maskB += mask_pitch;
			maskF += mask_pitch;
		}
	}
	else if (mode == 4) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				const PixelType *pRef_ = (const PixelType *)pRef;
				const PixelType *pSrc_ = (const PixelType *)pSrc;
				PixelType *pDst_ = (PixelType *)pDst;
				double f = (maskF[w] * static_cast<double>(pMCB_[w]) + (255. - maskF[w]) * pMCF_[w] + 255.) / 256.;
				double b = (maskB[w] * static_cast<double>(pMCF_[w]) + (255. - maskB[w]) * pMCB_[w] + 255.) / 256.;
				double avg = (static_cast<double>(pRef_[w]) * time256 + pSrc_[w] * (256. - time256) + 255.) / 256.;
				double m = (b * time256 + f * (256. - time256)) / 256.;
				pDst_[w] = static_cast<PixelType>((avg * pOcc[w] + m * (255. - pOcc[w]) + 255.) / 256.);
			}
			pDst += dst_pitch;
			pMCB += MCB_pitch;
			pMCF += MCF_pitch;
			pRef += ref_pitch;
			pSrc += src_pitch;
			maskB += mask_pitch;
			maskF += mask_pitch;
			pOcc += mask_pitch;
		}
	}
	else if (mode == 5) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				PixelType *pDst_ = (PixelType *)pDst;
				pDst_[w] = pOcc[w];
			}
			pDst += dst_pitch;
			pOcc += mask_pitch;
		}
	}
}

static void ResultBlock(uint8_t *pDst, int32_t dst_pitch, const uint8_t * pMCB, int32_t MCB_pitch, const uint8_t * pMCF, int32_t MCF_pitch,
	const uint8_t * pRef, int32_t ref_pitch, const uint8_t * pSrc, int32_t src_pitch, uint8_t *maskB, int32_t mask_pitch, uint8_t *maskF,
	uint8_t *pOcc, int32_t nBlkSizeX, int32_t nBlkSizeY, int32_t time256, int32_t mode) {
	RealResultBlock<float>(pDst, dst_pitch, pMCB, MCB_pitch, pMCF, MCF_pitch, pRef, ref_pitch, pSrc, src_pitch, maskB, mask_pitch, maskF, pOcc, nBlkSizeX, nBlkSizeY, time256, mode);
}

static const VSFrameRef *VS_CC mvblockfpsGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData *d = reinterpret_cast<MVBlockFPSData *>(*instanceData);
	if (activationReason == arInitial) {
		int32_t off = d->mvClipB->GetDeltaFrame();
		int32_t nleft = (int32_t)(n * d->fa / d->fb);
		int32_t nright = nleft + off;
		int32_t time256 = int32_t((double(n) * double(d->fa) / double(d->fb) - nleft) * 256 + 0.5);
		if (off > 1)
			time256 = time256 / off;
		if (time256 == 0) {
			vsapi->requestFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx);
			return 0;
		}
		else if (time256 == 256) {
			vsapi->requestFrameFilter(d->vi.numFrames ? VSMIN(nright, d->vi.numFrames - 1) : nright, d->node, frameCtx);
			return 0;
		}
		if ((nleft < d->vi.numFrames && nright < d->vi.numFrames) || !d->vi.numFrames) {
			vsapi->requestFrameFilter(nright, d->mvfw, frameCtx);
			vsapi->requestFrameFilter(nleft, d->mvbw, frameCtx);

			vsapi->requestFrameFilter(nleft, d->super, frameCtx);
			vsapi->requestFrameFilter(nright, d->super, frameCtx);
		}
		vsapi->requestFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx);
		if (d->blend)
			vsapi->requestFrameFilter(d->vi.numFrames ? VSMIN(nright, d->vi.numFrames - 1) : nright, d->node, frameCtx);

	}
	else if (activationReason == arAllFramesReady) {
		int32_t nleft = (int32_t)(n * d->fa / d->fb);
		int32_t time256 = int32_t((double(n)*double(d->fa) / double(d->fb) - nleft) * 256 + 0.5);
		int32_t off = d->mvClipB->GetDeltaFrame();
		if (off > 1)
			time256 = time256 / off;
		int32_t nright = nleft + off;
		if (time256 == 0)
			return vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx);
		else if (time256 == 256)
			return vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nright, d->vi.numFrames - 1) : nright, d->node, frameCtx);
		MVClipBalls ballsF(d->mvClipF, vsapi);
		MVClipBalls ballsB(d->mvClipB, vsapi);
		bool isUsableF = false;
		bool isUsableB = false;
		if ((nleft < d->vi.numFrames && nright < d->vi.numFrames) || !d->vi.numFrames) {
			const VSFrameRef *mvF = vsapi->getFrameFilter(nright, d->mvfw, frameCtx);
			ballsF.Update(mvF);
			isUsableF = ballsF.IsUsable();
			vsapi->freeFrame(mvF);
			const VSFrameRef *mvB = vsapi->getFrameFilter(nleft, d->mvbw, frameCtx);
			ballsB.Update(mvB);
			isUsableB = ballsB.IsUsable();
			vsapi->freeFrame(mvB);
		}
		const int32_t nWidth = d->bleh->nWidth;
		const int32_t nHeight = d->bleh->nHeight;
		const int32_t nWidthUV = d->nWidthUV;
		const int32_t nHeightUV = d->nHeightUV;
		const int32_t nHeightP = d->nHeightP;
		const int32_t nHeightPUV = d->nHeightPUV;
		const int32_t mode = d->mode;
		const int32_t thres = d->thres;
		const bool blend = d->blend;
		const int32_t xRatioUV = d->bleh->xRatioUV;
		const int32_t yRatioUV = d->bleh->yRatioUV;
		const int32_t nBlkX = d->bleh->nBlkX;
		const int32_t nBlkY = d->bleh->nBlkY;
		const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
		const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
		const int32_t nPel = d->bleh->nPel;
		const int32_t nPitchY = d->nPitchY;
		const int32_t nPitchUV = d->nPitchUV;
		const int32_t nBlkXP = d->nBlkXP;
		const int32_t nBlkYP = d->nBlkYP;
		SimpleResize *upsizer = d->upsizer;
		SimpleResize *upsizerUV = d->upsizerUV;
		const uint8_t *OnesBlock = d->OnesBlock;
		const int32_t nSuperHPad = d->nSuperHPad;
		const int32_t nSuperVPad = d->nSuperVPad;
		const int32_t nSuperModeYUV = d->nSuperModeYUV;
		const int32_t nSuperLevels = d->nSuperLevels;
		const int32_t nSuperPel = d->nSuperPel;
		const int32_t bytesPerSample = d->supervi->format->bytesPerSample;
		if (isUsableB && isUsableF) {
			uint8_t *pDst[3];
			const uint8_t *pRef[3], *pSrc[3];
			int32_t nDstPitches[3], nRefPitches[3], nSrcPitches[3];
			const VSFrameRef *src = vsapi->getFrameFilter(nleft, d->super, frameCtx);
			const VSFrameRef *ref = vsapi->getFrameFilter(nright, d->super, frameCtx);
			VSFrameRef *dst = vsapi->newVideoFrame(d->vi.format, d->vi.width, d->vi.height, src, core);
			for (int32_t i = 0; i < d->supervi->format->numPlanes; i++) {
				pDst[i] = vsapi->getWritePtr(dst, i);
				pRef[i] = vsapi->getReadPtr(ref, i);
				pSrc[i] = vsapi->getReadPtr(src, i);
				nDstPitches[i] = vsapi->getStride(dst, i);
				nRefPitches[i] = vsapi->getStride(ref, i);
				nSrcPitches[i] = vsapi->getStride(src, i);
			}
			MVGroupOfFrames *pRefBGOF = new MVGroupOfFrames(nSuperLevels, nWidth, nHeight, nSuperPel, nSuperHPad, nSuperVPad, nSuperModeYUV, xRatioUV, yRatioUV);
			MVGroupOfFrames *pRefFGOF = new MVGroupOfFrames(nSuperLevels, nWidth, nHeight, nSuperPel, nSuperHPad, nSuperVPad, nSuperModeYUV, xRatioUV, yRatioUV);
			pRefBGOF->Update(nSuperModeYUV, (uint8_t*)pRef[0], nRefPitches[0], (uint8_t*)pRef[1], nRefPitches[1], (uint8_t*)pRef[2], nRefPitches[2]);
			pRefFGOF->Update(nSuperModeYUV, (uint8_t*)pSrc[0], nSrcPitches[0], (uint8_t*)pSrc[1], nSrcPitches[1], (uint8_t*)pSrc[2], nSrcPitches[2]);
			MVPlane *pPlanesB[3] = { 0 };
			MVPlane *pPlanesF[3] = { 0 };
			MVPlaneSet planes[3] = { YPLANE, UPLANE, VPLANE };
			for (int32_t plane = 0; plane < d->supervi->format->numPlanes; plane++) {
				if (nSuperModeYUV & planes[plane]) {
					pPlanesB[plane] = pRefBGOF->GetFrame(0)->GetPlane(planes[plane]);
					pPlanesF[plane] = pRefFGOF->GetFrame(0)->GetPlane(planes[plane]);
				}
			}
			uint8_t *MaskFullYB = new uint8_t[nHeightP * nPitchY];
			uint8_t *MaskFullYF = new uint8_t[nHeightP * nPitchY];
			uint8_t *MaskOccY = new uint8_t[nHeightP * nPitchY];
			uint8_t *MaskFullUVB = nullptr;
			uint8_t *MaskFullUVF = nullptr;
			uint8_t *MaskOccUV = nullptr;
			if (nSuperModeYUV & UVPLANES) {
				MaskFullUVB = new uint8_t[nHeightPUV * nPitchUV];
				MaskFullUVF = new uint8_t[nHeightPUV * nPitchUV];
				MaskOccUV = new uint8_t[nHeightPUV * nPitchUV];
			}
			uint8_t *smallMaskB = nullptr;
			uint8_t *smallMaskF = nullptr;
			uint8_t *smallMaskO = nullptr;
			memset(MaskFullYB, 0, nHeightP * nPitchY);
			memset(MaskFullYF, 0, nHeightP * nPitchY);
			int32_t blocks = d->mvClipB->GetBlkCount();
			int32_t maxoffset = nPitchY * (nHeightP - nBlkSizeY) - nBlkSizeX;
			if (mode == 3 || mode == 4 || mode == 5) {
				smallMaskB = new uint8_t[nBlkXP * nBlkYP];
				smallMaskF = new uint8_t[nBlkXP * nBlkYP];
				smallMaskO = new uint8_t[nBlkXP * nBlkYP];
				for (int32_t i = 0; i < blocks; i++) {
					const FakeBlockData &blockF = ballsF.GetBlock(0, i);
					int32_t offset = blockF.GetX() - ((blockF.GetMV().x * time256) >> 8) / nPel + (blockF.GetY() - ((blockF.GetMV().y * time256) >> 8) / nPel) * nPitchY;
					if (offset >= 0 && offset < maxoffset)
						d->BLITLUMA(MaskFullYF + offset, nPitchY, OnesBlock, nBlkSizeX);
				}
				for (int32_t i = 0; i < blocks; i++) {
					const FakeBlockData &blockB = ballsB.GetBlock(0, i);
					int32_t offset = blockB.GetX() - ((blockB.GetMV().x * (256 - time256)) >> 8) / nPel + (blockB.GetY() - ((blockB.GetMV().y * (256 - time256)) >> 8) / nPel) * nPitchY;
					if (offset >= 0 && offset < maxoffset)
						d->BLITLUMA(MaskFullYB + offset, nPitchY, OnesBlock, nBlkSizeX);
				}
				MakeSmallMask(MaskFullYF, nPitchY, smallMaskF, nBlkXP, nBlkYP, nBlkSizeX, nBlkSizeY, thres);
				InflateMask(smallMaskF, nBlkXP, nBlkYP);
				upsizer->Resize(MaskFullYF, nPitchY, smallMaskF, nBlkXP);
				MakeSmallMask(MaskFullYB, nPitchY, smallMaskB, nBlkXP, nBlkYP, nBlkSizeX, nBlkSizeY, thres);
				InflateMask(smallMaskB, nBlkXP, nBlkYP);
				upsizer->Resize(MaskFullYB, nPitchY, smallMaskB, nBlkXP);
				if (nSuperModeYUV & UVPLANES) {
					upsizerUV->Resize(MaskFullUVF, nPitchUV, smallMaskF, nBlkXP);
					upsizerUV->Resize(MaskFullUVB, nPitchUV, smallMaskB, nBlkXP);
				}
			}
			if (mode == 4 || mode == 5) {
				MultMasks(smallMaskF, smallMaskB, smallMaskO, nBlkXP, nBlkYP);
				InflateMask(smallMaskO, nBlkXP, nBlkYP);
				upsizer->Resize(MaskOccY, nPitchY, smallMaskO, nBlkXP);
				if (nSuperModeYUV & UVPLANES)
					upsizerUV->Resize(MaskOccUV, nPitchUV, smallMaskO, nBlkXP);
			}
			uint8_t * pMaskFullYB = MaskFullYB;
			uint8_t * pMaskFullYF = MaskFullYF;
			uint8_t * pMaskFullUVB = MaskFullUVB;
			uint8_t * pMaskFullUVF = MaskFullUVF;
			uint8_t * pMaskOccY = MaskOccY;
			uint8_t * pMaskOccUV = MaskOccUV;
			pSrc[0] += nSuperHPad * bytesPerSample + nSrcPitches[0] * nSuperVPad;
			pRef[0] += nSuperHPad * bytesPerSample + nRefPitches[0] * nSuperVPad;
			if (nSuperModeYUV & UVPLANES) {
				pSrc[1] += (nSuperHPad >> 1) * bytesPerSample + nSrcPitches[1] * (nSuperVPad >> 1);
				pSrc[2] += (nSuperHPad >> 1) * bytesPerSample + nSrcPitches[2] * (nSuperVPad >> 1);
				pRef[1] += (nSuperHPad >> 1) * bytesPerSample + nRefPitches[1] * (nSuperVPad >> 1);
				pRef[2] += (nSuperHPad >> 1) * bytesPerSample + nRefPitches[2] * (nSuperVPad >> 1);
			}
			for (int32_t i = 0; i < blocks; i++) {
				const FakeBlockData &blockB = ballsB.GetBlock(0, i);
				const FakeBlockData &blockF = ballsF.GetBlock(0, i);
				ResultBlock(pDst[0], nDstPitches[0],
					pPlanesB[0]->GetPointer(blockB.GetX() * nPel + ((blockB.GetMV().x * (256 - time256)) >> 8), blockB.GetY() * nPel + ((blockB.GetMV().y * (256 - time256)) >> 8)),
					pPlanesB[0]->GetPitch(),
					pPlanesF[0]->GetPointer(blockF.GetX() * nPel + ((blockF.GetMV().x * time256) >> 8), blockF.GetY() * nPel + ((blockF.GetMV().y * time256) >> 8)),
					pPlanesF[0]->GetPitch(),
					pRef[0], nRefPitches[0],
					pSrc[0], nSrcPitches[0],
					pMaskFullYB, nPitchY,
					pMaskFullYF, pMaskOccY,
					nBlkSizeX, nBlkSizeY, time256, mode);
				if (nSuperModeYUV & UVPLANES) {
					ResultBlock(pDst[1], nDstPitches[1],
						pPlanesB[1]->GetPointer((blockB.GetX() * nPel + ((blockB.GetMV().x * (256 - time256)) >> 8)) / xRatioUV, (blockB.GetY() * nPel + ((blockB.GetMV().y * (256 - time256)) >> 8)) / yRatioUV),
						pPlanesB[1]->GetPitch(),
						pPlanesF[1]->GetPointer((blockF.GetX() * nPel + ((blockF.GetMV().x * time256) >> 8)) / xRatioUV, (blockF.GetY() * nPel + ((blockF.GetMV().y * time256) >> 8)) / yRatioUV),
						pPlanesF[1]->GetPitch(),
						pRef[1], nRefPitches[1],
						pSrc[1], nSrcPitches[1],
						pMaskFullUVB, nPitchUV,
						pMaskFullUVF, pMaskOccUV,
						nBlkSizeX / xRatioUV, nBlkSizeY / yRatioUV, time256, mode);
					ResultBlock(pDst[2], nDstPitches[2],
						pPlanesB[2]->GetPointer((blockB.GetX() * nPel + ((blockB.GetMV().x * (256 - time256)) >> 8)) / xRatioUV, (blockB.GetY() * nPel + ((blockB.GetMV().y * (256 - time256)) >> 8)) / yRatioUV),
						pPlanesB[2]->GetPitch(),
						pPlanesF[2]->GetPointer((blockF.GetX() * nPel + ((blockF.GetMV().x * time256) >> 8)) / xRatioUV, (blockF.GetY() * nPel + ((blockF.GetMV().y * time256) >> 8)) / yRatioUV),
						pPlanesF[2]->GetPitch(),
						pRef[2], nRefPitches[2],
						pSrc[2], nSrcPitches[2],
						pMaskFullUVB, nPitchUV,
						pMaskFullUVF, pMaskOccUV,
						nBlkSizeX / xRatioUV, nBlkSizeY / yRatioUV, time256, mode);
				}
				pDst[0] += nBlkSizeX * bytesPerSample;
				pRef[0] += nBlkSizeX * bytesPerSample;
				pSrc[0] += nBlkSizeX * bytesPerSample;
				pMaskFullYB += nBlkSizeX;
				pMaskFullYF += nBlkSizeX;
				pMaskOccY += nBlkSizeX;
				if (nSuperModeYUV & UVPLANES) {
					pDst[1] += nBlkSizeX / xRatioUV * bytesPerSample;
					pDst[2] += nBlkSizeX / xRatioUV * bytesPerSample;
					pRef[1] += nBlkSizeX / xRatioUV * bytesPerSample;
					pRef[2] += nBlkSizeX / xRatioUV * bytesPerSample;
					pSrc[1] += nBlkSizeX / xRatioUV * bytesPerSample;
					pSrc[2] += nBlkSizeX / xRatioUV * bytesPerSample;
					pMaskFullUVB += nBlkSizeX / xRatioUV;
					pMaskFullUVF += nBlkSizeX / xRatioUV;
					pMaskOccUV += nBlkSizeX / xRatioUV;
				}
				if (!((i + 1) % nBlkX)) {
					Blend(pDst[0], pSrc[0], pRef[0], nBlkSizeY, nWidth - nBlkSizeX * nBlkX, nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
					pDst[0] += nBlkSizeY * nDstPitches[0] - nBlkSizeX * nBlkX * bytesPerSample;
					pRef[0] += nBlkSizeY * nRefPitches[0] - nBlkSizeX * nBlkX * bytesPerSample;
					pSrc[0] += nBlkSizeY * nSrcPitches[0] - nBlkSizeX * nBlkX * bytesPerSample;
					pMaskFullYB += nBlkSizeY * nPitchY - nBlkSizeX * nBlkX;
					pMaskFullYF += nBlkSizeY * nPitchY - nBlkSizeX * nBlkX;
					pMaskOccY += nBlkSizeY * nPitchY - nBlkSizeX * nBlkX;

					if (nSuperModeYUV & UVPLANES) {
						Blend(pDst[1], pSrc[1], pRef[1], nBlkSizeY / yRatioUV, nWidthUV - (nBlkSizeX / xRatioUV) * nBlkX, nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
						Blend(pDst[2], pSrc[2], pRef[2], nBlkSizeY / yRatioUV, nWidthUV - (nBlkSizeX / xRatioUV) * nBlkX, nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);

						pDst[1] += (nBlkSizeY / yRatioUV) * nDstPitches[1] - (nBlkSizeX / xRatioUV) * nBlkX * bytesPerSample;
						pDst[2] += (nBlkSizeY / yRatioUV) * nDstPitches[2] - (nBlkSizeX / xRatioUV) * nBlkX * bytesPerSample;
						pRef[1] += (nBlkSizeY / yRatioUV) * nRefPitches[1] - (nBlkSizeX / xRatioUV) * nBlkX * bytesPerSample;
						pRef[2] += (nBlkSizeY / yRatioUV) * nRefPitches[2] - (nBlkSizeX / xRatioUV) * nBlkX * bytesPerSample;
						pSrc[1] += (nBlkSizeY / yRatioUV) * nSrcPitches[1] - (nBlkSizeX / xRatioUV) * nBlkX * bytesPerSample;
						pSrc[2] += (nBlkSizeY / yRatioUV) * nSrcPitches[2] - (nBlkSizeX / xRatioUV) * nBlkX * bytesPerSample;
						pMaskFullUVB += (nBlkSizeY / yRatioUV) * nPitchUV - (nBlkSizeX / xRatioUV) * nBlkX;
						pMaskFullUVF += (nBlkSizeY / yRatioUV) * nPitchUV - (nBlkSizeX / xRatioUV) * nBlkX;
						pMaskOccUV += (nBlkSizeY / yRatioUV) * nPitchUV - (nBlkSizeX / xRatioUV) * nBlkX;
					}
				}
			}
			Blend(pDst[0], pSrc[0], pRef[0], nHeight - nBlkSizeY * nBlkY, nWidth, nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
			if (nSuperModeYUV & UVPLANES) {
				Blend(pDst[1], pSrc[1], pRef[1], nHeightUV - (nBlkSizeY / yRatioUV) * nBlkY, nWidthUV, nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
				Blend(pDst[2], pSrc[2], pRef[2], nHeightUV - (nBlkSizeY / yRatioUV) * nBlkY, nWidthUV, nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);
			}
			delete[] MaskFullYB;
			delete[] MaskFullYF;
			delete[] MaskOccY;
			if (nSuperModeYUV & UVPLANES) {
				delete[] MaskFullUVB;
				delete[] MaskFullUVF;
				delete[] MaskOccUV;
			}
			if (smallMaskB) {
				delete[] smallMaskB;
				delete[] smallMaskF;
				delete[] smallMaskO;
			}
			delete pRefBGOF;
			delete pRefFGOF;
			vsapi->freeFrame(src);
			vsapi->freeFrame(ref);
			return dst;
		}
		else {
			const VSFrameRef *src = vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx);
			if (blend) {
				uint8_t *pDst[3];
				const uint8_t *pRef[3], *pSrc[3];
				int32_t nDstPitches[3], nRefPitches[3], nSrcPitches[3];
				const VSFrameRef *ref = vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nright, d->vi.numFrames - 1) : nright, d->node, frameCtx);
				VSFrameRef *dst = vsapi->newVideoFrame(d->vi.format, d->vi.width, d->vi.height, src, core);
				for (int32_t i = 0; i < d->vi.format->numPlanes; i++) {
					pDst[i] = vsapi->getWritePtr(dst, i);
					pRef[i] = vsapi->getReadPtr(ref, i);
					pSrc[i] = vsapi->getReadPtr(src, i);
					nDstPitches[i] = vsapi->getStride(dst, i);
					nRefPitches[i] = vsapi->getStride(ref, i);
					nSrcPitches[i] = vsapi->getStride(src, i);
				}
				Blend(pDst[0], pSrc[0], pRef[0], nHeight, nWidth, nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
				if (nSuperModeYUV & UVPLANES) {
					Blend(pDst[1], pSrc[1], pRef[1], nHeightUV, nWidthUV, nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
					Blend(pDst[2], pSrc[2], pRef[2], nHeightUV, nWidthUV, nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);
				}
				vsapi->freeFrame(src);
				vsapi->freeFrame(ref);
				return dst;
			}
			else
				return src;
		}
	}
	return nullptr;
}

static void VS_CC mvblockfpsFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData *d = reinterpret_cast<MVBlockFPSData *>(instanceData);
	delete d->mvClipB;
	delete d->mvClipF;
	delete d->bleh;
	delete d->upsizer;
	if (d->supervi->format->colorFamily != cmGray)
		delete d->upsizerUV;
	delete[] d->OnesBlock;
	vsapi->freeNode(d->super);
	vsapi->freeNode(d->mvfw);
	vsapi->freeNode(d->mvbw);
	vsapi->freeNode(d->node);
	delete d;
}

static inline void setFPS(VSVideoInfo *vi, int64_t num, int64_t den) {
	if (num <= 0 || den <= 0) {
		vi->fpsNum = 0;
		vi->fpsDen = 1;
	}
	else {
		int64_t x = num;
		int64_t y = den;
		while (y) {
			int64_t t = x % y;
			x = y;
			y = t;
		}
		vi->fpsNum = num / x;
		vi->fpsDen = den / x;
	}
}

static void selectFunctions(MVBlockFPSData *d) {
	const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
	const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
	COPYFunction copys[33][33];
	copys[2][2] = Copy_C<2, 2, uint8_t>;
	copys[2][4] = Copy_C<2, 4, uint8_t>;
	copys[4][2] = Copy_C<4, 2, uint8_t>;
	copys[4][4] = Copy_C<4, 4, uint8_t>;
	copys[4][8] = Copy_C<4, 8, uint8_t>;
	copys[8][1] = Copy_C<8, 1, uint8_t>;
	copys[8][2] = Copy_C<8, 2, uint8_t>;
	copys[8][4] = Copy_C<8, 4, uint8_t>;
	copys[8][8] = Copy_C<8, 8, uint8_t>;
	copys[8][16] = Copy_C<8, 16, uint8_t>;
	copys[16][1] = Copy_C<16, 1, uint8_t>;
	copys[16][2] = Copy_C<16, 2, uint8_t>;
	copys[16][4] = Copy_C<16, 4, uint8_t>;
	copys[16][8] = Copy_C<16, 8, uint8_t>;
	copys[16][16] = Copy_C<16, 16, uint8_t>;
	copys[16][32] = Copy_C<16, 32, uint8_t>;
	copys[32][8] = Copy_C<32, 8, uint8_t>;
	copys[32][16] = Copy_C<32, 16, uint8_t>;
	copys[32][32] = Copy_C<32, 32, uint8_t>;
	d->BLITLUMA = copys[nBlkSizeX][nBlkSizeY];
}

static void VS_CC mvblockfpsCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData d;
	MVBlockFPSData *data;
	int err, thres_err;
	d.num = vsapi->propGetInt(in, "num", 0, &err);
	if (err)
		d.num = 25;
	d.den = vsapi->propGetInt(in, "den", 0, &err);
	if (err)
		d.den = 1;
	d.mode = int64ToIntS(vsapi->propGetInt(in, "mode", 0, &err));
	d.thres = int64ToIntS(vsapi->propGetInt(in, "thres", 0, &thres_err));
	d.blend = !!vsapi->propGetInt(in, "blend", 0, &err);
	if (err)
		d.blend = 1;
	d.thscd1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.thscd1 = MV_DEFAULT_SCD1;
	d.thscd2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.thscd2 = MV_DEFAULT_SCD2;
	if (d.mode < 0 || d.mode > 5) {
		vsapi->setError(out, "BlockFPS: mode must be between 0 and 5 (inclusive).");
		return;
	}
	d.super = vsapi->propGetNode(in, "super", 0, nullptr);
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("BlockFPS: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.super);
		return;
	}
	const VSMap *props = vsapi->getFramePropsRO(evil);
	int32_t evil_err[6];
	int32_t nHeightS = int64ToIntS(vsapi->propGetInt(props, "Super_height", 0, &evil_err[0]));
	d.nSuperHPad = int64ToIntS(vsapi->propGetInt(props, "Super_hpad", 0, &evil_err[1]));
	d.nSuperVPad = int64ToIntS(vsapi->propGetInt(props, "Super_vpad", 0, &evil_err[2]));
	d.nSuperPel = int64ToIntS(vsapi->propGetInt(props, "Super_pel", 0, &evil_err[3]));
	d.nSuperModeYUV = int64ToIntS(vsapi->propGetInt(props, "Super_modeyuv", 0, &evil_err[4]));
	d.nSuperLevels = int64ToIntS(vsapi->propGetInt(props, "Super_levels", 0, &evil_err[5]));
	vsapi->freeFrame(evil);
	for (int32_t i = 0; i < 6; i++)
		if (evil_err[i]) {
			vsapi->setError(out, "BlockFPS: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.super);
			return;
		}
	d.mvbw = vsapi->propGetNode(in, "mvbw", 0, nullptr);
	d.mvfw = vsapi->propGetNode(in, "mvfw", 0, nullptr);
	try {
		d.mvClipB = new MVClipDicks(d.mvbw, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, (std::string("BlockFPS: ") + e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvbw);
		vsapi->freeNode(d.mvfw);
		return;
	}
	try {
		d.mvClipF = new MVClipDicks(d.mvfw, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, (std::string("BlockFPS: ") + e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		return;
	}
	if (d.mvClipF->GetDeltaFrame() != d.mvClipB->GetDeltaFrame()) {
		vsapi->setError(out, "BlockFPS: mvbw and mvfw must be generated with the same delta.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	if (!d.mvClipB->IsBackward() || d.mvClipF->IsBackward()) {
		if (!d.mvClipB->IsBackward())
			vsapi->setError(out, "BlockFPS: mvbw must be generated with isb=True.");
		else
			vsapi->setError(out, "BlockFPS: mvfw must be generated with isb=False.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	try {
		d.bleh = new MVFilter(d.mvfw, "BlockFPS", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, (std::string("BlockFPS: ") + e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	if (thres_err)
		d.thres = d.bleh->nBlkSizeX * d.bleh->nBlkSizeY / 4;
	if (d.bleh->nOverlapX || d.bleh->nOverlapY) {
		vsapi->setError(out, "BlockFPS: Overlap must be 0.");
		delete d.bleh;
		delete d.mvClipB;
		delete d.mvClipF;
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		return;
	}
	try {
		d.bleh->CheckSimilarity(d.mvClipF, "mvfw");
		d.bleh->CheckSimilarity(d.mvClipB, "mvbw");
	}
	catch (MVException &e) {
		vsapi->setError(out, (std::string("BlockFPS: ") + e.what()).c_str());
		delete d.bleh;
		delete d.mvClipB;
		delete d.mvClipF;
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		return;
	}
	d.node = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = *vsapi->getVideoInfo(d.node);
	if (d.vi.fpsNum == 0 || d.vi.fpsDen == 0) {
		vsapi->setError(out, "BlockFPS: The input clip must have a frame rate. Invoke AssumeFPS if necessary.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		vsapi->freeNode(d.node);
		delete d.bleh;
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	int64_t numeratorOld = d.vi.fpsNum;
	int64_t denominatorOld = d.vi.fpsDen;
	int64_t numerator, denominator;
	if (d.num != 0 && d.den != 0) {
		numerator = d.num;
		denominator = d.den;
	}
	else {
		numerator = numeratorOld * 2;
		denominator = denominatorOld;
	}
	d.fa = denominator * numeratorOld;
	d.fb = numerator * denominatorOld;
	int64_t fgcd = gcd(d.fa, d.fb);
	d.fa /= fgcd;
	d.fb /= fgcd;
	setFPS(&d.vi, numerator, denominator);
	if (d.vi.numFrames)
		d.vi.numFrames = (int32_t)(1 + (d.vi.numFrames - 1) * d.fb / d.fa);
	d.supervi = vsapi->getVideoInfo(d.super);
	int32_t nSuperWidth = d.supervi->width;
	if (d.bleh->nHeight != nHeightS ||
		d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2 ||
		d.bleh->nWidth != d.vi.width ||
		d.bleh->nHeight != d.vi.height) {
		vsapi->setError(out, "BlockFPS: wrong source or super clip frame size.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		vsapi->freeNode(d.node);
		delete d.bleh;
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	d.nBlkXP = (d.bleh->nBlkX * (d.bleh->nBlkSizeX - d.bleh->nOverlapX) + d.bleh->nOverlapX < d.bleh->nWidth) ? d.bleh->nBlkX + 1 : d.bleh->nBlkX;
	d.nBlkYP = (d.bleh->nBlkY * (d.bleh->nBlkSizeY - d.bleh->nOverlapY) + d.bleh->nOverlapY < d.bleh->nHeight) ? d.bleh->nBlkY + 1 : d.bleh->nBlkY;
	d.nWidthP = d.nBlkXP * (d.bleh->nBlkSizeX - d.bleh->nOverlapX) + d.bleh->nOverlapX;
	d.nHeightP = d.nBlkYP * (d.bleh->nBlkSizeY - d.bleh->nOverlapY) + d.bleh->nOverlapY;
	d.nWidthPUV = d.nWidthP / d.bleh->xRatioUV;
	d.nHeightPUV = d.nHeightP / d.bleh->yRatioUV;
	d.nHeightUV = d.bleh->nHeight / d.bleh->yRatioUV;
	d.nWidthUV = d.bleh->nWidth / d.bleh->xRatioUV;
	d.nPitchY = (d.nWidthP + 15) & (~15);
	d.nPitchUV = (d.nWidthPUV + 15) & (~15);
	d.OnesBlock = new uint8_t[d.bleh->nBlkSizeX * d.bleh->nBlkSizeY + 32];
	memset(d.OnesBlock, 255, d.bleh->nBlkSizeX * d.bleh->nBlkSizeY);
	d.upsizer = new SimpleResize(d.nWidthP, d.nHeightP, d.nBlkXP, d.nBlkYP);
	if (d.nSuperModeYUV & UVPLANES)
		d.upsizerUV = new SimpleResize(d.nWidthPUV, d.nHeightPUV, d.nBlkXP, d.nBlkYP);
	selectFunctions(&d);
	data = new MVBlockFPSData;
	*data = d;
	vsapi->createFilter(in, out, "BlockFPS", mvblockfpsInit, mvblockfpsGetFrame, mvblockfpsFree, fmParallel, 0, data, core);
	VSNodeRef *node = vsapi->propGetNode(out, "clip", 0, nullptr);
	VSMap *args = vsapi->createMap();
	vsapi->propSetNode(args, "clip", node, paReplace);
	vsapi->freeNode(node);
	vsapi->propSetInt(args, "fpsnum", d.vi.fpsNum, paReplace);
	vsapi->propSetInt(args, "fpsden", d.vi.fpsDen, paReplace);
	VSPlugin *stdPlugin = vsapi->getPluginById("com.vapoursynth.std", core);
	VSMap *ret = vsapi->invoke(stdPlugin, "AssumeFPS", args);
	const char *error = vsapi->getError(ret);
	if (error) {
		vsapi->setError(out, std::string("BlockFPS: Failed to invoke AssumeFPS. Error message: ").append(error).c_str());
		vsapi->freeMap(args);
		vsapi->freeMap(ret);
		return;
	}
	node = vsapi->propGetNode(ret, "clip", 0, nullptr);
	vsapi->freeMap(ret);
	vsapi->clearMap(args);
	vsapi->propSetNode(args, "clip", node, paReplace);
	vsapi->freeNode(node);
	ret = vsapi->invoke(stdPlugin, "Cache", args);
	vsapi->freeMap(args);
	error = vsapi->getError(ret);
	if (error) {
		vsapi->setError(out, std::string("BlockFPS: Failed to invoke Cache. Error message: ").append(error).c_str());
		vsapi->freeMap(ret);
		return;
	}
	node = vsapi->propGetNode(ret, "clip", 0, nullptr);
	vsapi->freeMap(ret);
	vsapi->propSetNode(out, "clip", node, paReplace);
	vsapi->freeNode(node);
}

void mvblockfpsRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("BlockFPS",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"num:int:opt;"
		"den:int:opt;"
		"mode:int:opt;"
		"thres:int:opt;"
		"blend:int:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvblockfpsCreate, 0, plugin);
}
