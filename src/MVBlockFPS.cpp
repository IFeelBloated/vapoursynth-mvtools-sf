#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "Overlap.h"
#include "VapourSynth.h"
#include "VSHelper.h"
#include "CopyCode.h"
#include "CommonFunctions.h"
#include "MaskFun.h"
#include "MVFilter.h"
#include "SimpleResize.hpp"

struct MVBlockFPSData {
	VSNodeRef *node;
	VSVideoInfo vi;
	const VSVideoInfo *oldvi;
	const VSVideoInfo *supervi;
	VSNodeRef *super;
	VSNodeRef *mvbw;
	VSNodeRef *mvfw;
	int64_t num, den;
	int32_t mode;
	double ml;
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
	SimpleResize<double> *upsizer;
	SimpleResize<double> *upsizerUV;
	int64_t fa, fb;
	int32_t dstTempPitch;
	int32_t dstTempPitchUV;
	int32_t nBlkPitch;
	OverlapWindows *OverWins;
	OverlapWindows *OverWinsUV;
	OverlapsFunction OVERSLUMA;
	OverlapsFunction OVERSCHROMA;
	ToPixelsFunction ToPixels;
};

static void VS_CC mvblockfpsInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData *d = reinterpret_cast<MVBlockFPSData *>(*instanceData);
	vsapi->setVideoInfo(&d->vi, 1, node);
}

static void MultMasks(double *smallmaskF, double *smallmaskB, double *smallmaskO, int32_t nBlkX, int32_t nBlkY) {
	for (auto j = 0; j < nBlkY; ++j) {
		for (auto i = 0; i < nBlkX; ++i)
			smallmaskO[i] = (static_cast<int64_t>(smallmaskF[i]) * smallmaskB[i]) / 255;
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
static void RealResultBlock(uint8_t *pDst, int32_t dst_pitch, const uint8_t * pMCB, int32_t MCB_pitch, const uint8_t *pMCF, int32_t MCF_pitch,
	const uint8_t *pRef, int32_t ref_pitch, const uint8_t *pSrc, int32_t src_pitch, double *maskB, int32_t mask_pitch, double *maskF,
	double *pOcc, int32_t nBlkSizeX, int32_t nBlkSizeY, int32_t time256, int32_t mode) {
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
	else if (mode == 3 || mode == 6) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				PixelType *pDst_ = (PixelType *)pDst;
				pDst_[w] = static_cast<PixelType>((((maskB[w] * pMCF_[w] + (255 - maskB[w]) * pMCB_[w]) / 256) * time256 +
					((maskF[w] * pMCB_[w] + (255 - maskF[w]) * pMCF_[w]) / 256) * (256 - time256)) / 256);
			}
			pDst += dst_pitch;
			pMCB += MCB_pitch;
			pMCF += MCF_pitch;
			maskB += mask_pitch;
			maskF += mask_pitch;
		}
	}
	else if (mode == 4 || mode == 7) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				const PixelType *pMCB_ = (const PixelType *)pMCB;
				const PixelType *pMCF_ = (const PixelType *)pMCF;
				const PixelType *pRef_ = (const PixelType *)pRef;
				const PixelType *pSrc_ = (const PixelType *)pSrc;
				PixelType *pDst_ = (PixelType *)pDst;
				double f = (maskF[w] * pMCB_[w] + (255. - maskF[w]) * pMCF_[w]) / 256.;
				double b = (maskB[w] * pMCF_[w] + (255. - maskB[w]) * pMCB_[w]) / 256.;
				double avg = (static_cast<double>(pRef_[w]) * time256 + pSrc_[w] * (256. - time256)) / 256.;
				double m = (b * time256 + f * (256. - time256)) / 256.;
				pDst_[w] = static_cast<PixelType>((avg * pOcc[w] + m * (255. - pOcc[w])) / 256.);
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
	else if (mode == 5 || mode == 8) {
		for (int32_t h = 0; h < nBlkSizeY; h++) {
			for (int32_t w = 0; w < nBlkSizeX; w++) {
				PixelType *pDst_ = (PixelType *)pDst;
				pDst_[w] = static_cast<PixelType>(pOcc[w] / 255.);
			}
			pDst += dst_pitch;
			pOcc += mask_pitch;
		}
	}
}

static void ResultBlock(uint8_t *pDst, int32_t dst_pitch, const uint8_t * pMCB, int32_t MCB_pitch, const uint8_t * pMCF, int32_t MCF_pitch,
	const uint8_t * pRef, int32_t ref_pitch, const uint8_t * pSrc, int32_t src_pitch, double *maskB, int32_t mask_pitch, double *maskF,
	double *pOcc, int32_t nBlkSizeX, int32_t nBlkSizeY, int32_t time256, int32_t mode) {
	RealResultBlock<float>(pDst, dst_pitch, pMCB, MCB_pitch, pMCF, MCF_pitch, pRef, ref_pitch, pSrc, src_pitch, maskB, mask_pitch, maskF, pOcc, nBlkSizeX, nBlkSizeY, time256, mode);
}

static const VSFrameRef *VS_CC mvblockfpsGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData *d = reinterpret_cast<MVBlockFPSData *>(*instanceData);
	if (activationReason == arInitial) {
		int32_t off = d->mvClipB->GetDeltaFrame();
		int32_t nleft = static_cast<int32_t>(n * d->fa / d->fb);
		int32_t nright = nleft + off;
		int32_t time256 = static_cast<int32_t>((n * d->fa / static_cast<double>(d->fb) - nleft) * 256 + 0.5);
		if (off > 1)
			time256 = time256 / off;
		if (time256 == 0) {
			vsapi->requestFrameFilter(VSMIN(nleft, d->oldvi->numFrames - 1), d->node, frameCtx);
			return 0;
		}
		else if (time256 == 256) {
			vsapi->requestFrameFilter(VSMIN(nright, d->oldvi->numFrames - 1), d->node, frameCtx);
			return 0;
		}
		if (nleft < d->oldvi->numFrames && nright < d->oldvi->numFrames) {
			vsapi->requestFrameFilter(nright, d->mvfw, frameCtx);
			vsapi->requestFrameFilter(nleft, d->mvbw, frameCtx);

			vsapi->requestFrameFilter(nleft, d->super, frameCtx);
			vsapi->requestFrameFilter(nright, d->super, frameCtx);
		}
		vsapi->requestFrameFilter(VSMIN(nleft, d->oldvi->numFrames - 1), d->node, frameCtx);
		if (d->blend)
			vsapi->requestFrameFilter(VSMIN(nright, d->oldvi->numFrames - 1), d->node, frameCtx);

	}
	else if (activationReason == arAllFramesReady) {
		int32_t nleft = static_cast<int32_t>(n * d->fa / d->fb);
		int32_t time256 = static_cast<int32_t>((n * d->fa / static_cast<double>(d->fb) - nleft) * 256 + 0.5);
		int32_t off = d->mvClipB->GetDeltaFrame();
		if (off > 1)
			time256 = time256 / off;
		int32_t nright = nleft + off;
		if (time256 == 0)
			return vsapi->getFrameFilter(VSMIN(nleft, d->oldvi->numFrames - 1), d->node, frameCtx);
		else if (time256 == 256)
			return vsapi->getFrameFilter(VSMIN(nright, d->oldvi->numFrames - 1), d->node, frameCtx);
		MVClipBalls ballsF(d->mvClipF, vsapi);
		MVClipBalls ballsB(d->mvClipB, vsapi);
		bool isUsableF = false;
		bool isUsableB = false;
		if (nleft < d->oldvi->numFrames && nright < d->oldvi->numFrames) {
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
		const double ml = d->ml;
		const bool blend = d->blend;
		const int32_t xRatioUV = d->bleh->xRatioUV;
		const int32_t yRatioUV = d->bleh->yRatioUV;
		const int32_t nBlkX = d->bleh->nBlkX;
		const int32_t nBlkY = d->bleh->nBlkY;
		const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
		const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
		const int32_t nOverlapX = d->bleh->nOverlapX;
		const int32_t nOverlapY = d->bleh->nOverlapY;
		const int32_t nPel = d->bleh->nPel;
		const int32_t nPitchY = d->nPitchY;
		const int32_t nPitchUV = d->nPitchUV;
		const int32_t nBlkXP = d->nBlkXP;
		const int32_t nBlkYP = d->nBlkYP;
		const int32_t nWidth_B = nBlkX * (nBlkSizeX - nOverlapX) + nOverlapX;
		const int32_t nHeight_B = nBlkY * (nBlkSizeY - nOverlapY) + nOverlapY;
		SimpleResize<double> *upsizer = d->upsizer;
		SimpleResize<double> *upsizerUV = d->upsizerUV;
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
			for (int i = 0; i < d->supervi->format->numPlanes; ++i) {
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
			for (int plane = 0; plane < d->supervi->format->numPlanes; ++plane) {
				if (nSuperModeYUV & planes[plane]) {
					pPlanesB[plane] = pRefBGOF->GetFrame(0)->GetPlane(planes[plane]);
					pPlanesF[plane] = pRefFGOF->GetFrame(0)->GetPlane(planes[plane]);
				}
			}
			auto *MaskFullYB = new double[nHeightP * nPitchY];
			auto *MaskFullYF = new double[nHeightP * nPitchY];
			auto *MaskOccY = new double[nHeightP * nPitchY];
			double *MaskFullUVB = nullptr;
			double *MaskFullUVF = nullptr;
			double *MaskOccUV = nullptr;
			if (nSuperModeYUV & UVPLANES) {
				MaskFullUVB = new double[nHeightPUV * nPitchUV];
				MaskFullUVF = new double[nHeightPUV * nPitchUV];
				MaskOccUV = new double[nHeightPUV * nPitchUV];
			}
			double *smallMaskB = nullptr;
			double *smallMaskF = nullptr;
			double *smallMaskO = nullptr;
			for (auto i = 0; i < nHeightP * nPitchY; ++i) {
				MaskFullYB[i] = 0.;
				MaskFullYF[i] = 0.;
			}
			int32_t blocks = d->mvClipB->GetBlkCount();
			if (mode >= 3 && mode <= 8) {
				smallMaskB = new double[nBlkXP * nBlkYP];
				smallMaskF = new double[nBlkXP * nBlkYP];
				smallMaskO = new double[nBlkXP * nBlkYP];
				if (mode <= 5) {
					MakeVectorOcclusionMaskTime(&ballsF, false, nBlkX, nBlkY, ml, 1.0, nPel, smallMaskF, nBlkXP, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
					MakeVectorOcclusionMaskTime(&ballsB, true, nBlkX, nBlkY, ml, 1.0, nPel, smallMaskB, nBlkXP, 256 - time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
				}
				else {
					MakeSADMaskTime(&ballsF, nBlkX, nBlkY, 4.0 / (ml * nBlkSizeX * nBlkSizeY), 1.0, nPel, smallMaskF, nBlkXP, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
					MakeSADMaskTime(&ballsB, nBlkX, nBlkY, 4.0 / (ml * nBlkSizeX * nBlkSizeY), 1.0, nPel, smallMaskB, nBlkXP, 256 - time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
				}
				if (nBlkXP > nBlkX)
					for (int j = 0; j < nBlkY; ++j) {
						smallMaskF[j * nBlkXP + nBlkX] = smallMaskF[j * nBlkXP + nBlkX - 1];
						smallMaskB[j * nBlkXP + nBlkX] = smallMaskB[j * nBlkXP + nBlkX - 1];
					}
				if (nBlkYP > nBlkY)
					for (int i = 0; i < nBlkXP; ++i) {
						smallMaskF[nBlkXP * nBlkY + i] = smallMaskF[nBlkXP * (nBlkY - 1) + i];
						smallMaskB[nBlkXP * nBlkY + i] = smallMaskB[nBlkXP * (nBlkY - 1) + i];
					}
				upsizer->Resize(MaskFullYF, nPitchY, smallMaskF, nBlkXP);
				upsizer->Resize(MaskFullYB, nPitchY, smallMaskB, nBlkXP);
				if (nSuperModeYUV & UVPLANES) {
					upsizerUV->Resize(MaskFullUVF, nPitchUV, smallMaskF, nBlkXP);
					upsizerUV->Resize(MaskFullUVB, nPitchUV, smallMaskB, nBlkXP);
				}
			}
			if (mode == 4 || mode == 5 || mode == 7 || mode == 8) {
				MultMasks(smallMaskF, smallMaskB, smallMaskO, nBlkXP, nBlkYP);
				upsizer->Resize(MaskOccY, nPitchY, smallMaskO, nBlkXP);
				if (nSuperModeYUV & UVPLANES)
					upsizerUV->Resize(MaskOccUV, nPitchUV, smallMaskO, nBlkXP);
			}
			auto pMaskFullYB = MaskFullYB;
			auto pMaskFullYF = MaskFullYF;
			auto pMaskFullUVB = MaskFullUVB;
			auto pMaskFullUVF = MaskFullUVF;
			auto pMaskOccY = MaskOccY;
			auto pMaskOccUV = MaskOccUV;
			pSrc[0] += nSuperHPad * bytesPerSample + nSrcPitches[0] * nSuperVPad;
			pRef[0] += nSuperHPad * bytesPerSample + nRefPitches[0] * nSuperVPad;
			if (nSuperModeYUV & UVPLANES) {
				pSrc[1] += (nSuperHPad >> 1) * bytesPerSample + nSrcPitches[1] * (nSuperVPad >> 1);
				pSrc[2] += (nSuperHPad >> 1) * bytesPerSample + nSrcPitches[2] * (nSuperVPad >> 1);
				pRef[1] += (nSuperHPad >> 1) * bytesPerSample + nRefPitches[1] * (nSuperVPad >> 1);
				pRef[2] += (nSuperHPad >> 1) * bytesPerSample + nRefPitches[2] * (nSuperVPad >> 1);
			}
			if (nOverlapX == 0 && nOverlapY == 0) {
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
			}
			else { // overlap
				   // blend rest right with time weight
				OverlapWindows *OverWins = d->OverWins;
				OverlapWindows *OverWinsUV = d->OverWinsUV;
				Blend(pDst[0] + nWidth_B * bytesPerSample,
					pSrc[0] + nWidth_B * bytesPerSample,
					pRef[0] + nWidth_B * bytesPerSample,
					nHeight_B,
					nWidth - nWidth_B,
					nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
				if (nSuperModeYUV & UVPLANES) {
					Blend(pDst[1] + nWidth_B / xRatioUV * bytesPerSample,
						pSrc[1] + nWidth_B / xRatioUV * bytesPerSample,
						pRef[1] + nWidth_B / xRatioUV * bytesPerSample,
						nHeight_B / yRatioUV,
						nWidthUV - nWidth_B / xRatioUV,
						nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
					Blend(pDst[2] + nWidth_B / xRatioUV * bytesPerSample,
						pSrc[2] + nWidth_B / xRatioUV * bytesPerSample,
						pRef[2] + nWidth_B / xRatioUV * bytesPerSample,
						nHeight_B / yRatioUV,
						nWidthUV - nWidth_B / xRatioUV,
						nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);
				}

				// blend rest bottom with time weight
				Blend(pDst[0] + nHeight_B * nDstPitches[0],
					pSrc[0] + nHeight_B * nSrcPitches[0],
					pRef[0] + nHeight_B * nRefPitches[0],
					nHeight - nHeight_B,
					nWidth,
					nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
				if (nSuperModeYUV & UVPLANES) {
					Blend(pDst[1] + nDstPitches[1] * nHeight_B / yRatioUV,
						pSrc[1] + nSrcPitches[1] * nHeight_B / yRatioUV,
						pRef[1] + nRefPitches[1] * nHeight_B / yRatioUV,
						nHeightUV - nHeight_B / yRatioUV,
						nWidthUV,
						nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
					Blend(pDst[2] + nDstPitches[2] * nHeight_B / yRatioUV,
						pSrc[2] + nSrcPitches[2] * nHeight_B / yRatioUV,
						pRef[2] + nRefPitches[2] * nHeight_B / yRatioUV,
						nHeightUV - nHeight_B / yRatioUV,
						nWidthUV,
						nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);
				}

				int32_t dstTempPitch = d->dstTempPitch;
				int32_t dstTempPitchUV = d->dstTempPitchUV;
				int32_t nBlkPitch = d->nBlkPitch;

				auto *DstTemp = new uint8_t[dstTempPitch * nHeight];
				uint8_t *DstTempU = nullptr;
				uint8_t *DstTempV = nullptr;
				if (nSuperModeYUV & UVPLANES) {
					DstTempU = new uint8_t[dstTempPitchUV * nHeightUV];
					DstTempV = new uint8_t[dstTempPitchUV * nHeightUV];
				}

				memset(DstTemp, 0, nHeight_B * dstTempPitch);
				if (nSuperModeYUV & UVPLANES) {
					memset(DstTempU, 0, nHeight_B / yRatioUV * dstTempPitchUV);
					memset(DstTempV, 0, nHeight_B / yRatioUV * dstTempPitchUV);
				}

				uint8_t *pDstTemp = DstTemp;
				uint8_t *pDstTempU = DstTempU;
				uint8_t *pDstTempV = DstTempV;

				uint8_t *TmpBlock = new uint8_t[nBlkSizeY * nBlkPitch];

				for (int by = 0; by < nBlkY; ++by) {
					int32_t wby = ((by + nBlkY - 3) / (nBlkY - 2)) * 3;
					int32_t xx = 0;
					int32_t xxUV = 0;
					for (int bx = 0; bx < nBlkX; bx++) {
						int32_t wbx = (bx + nBlkX - 3) / (nBlkX - 2);
						int32_t *winOver = OverWins->GetWindow(wby + wbx);
						int32_t *winOverUV = nullptr;
						if (nSuperModeYUV & UVPLANES)
							winOverUV = OverWinsUV->GetWindow(wby + wbx);
						int32_t i = by * nBlkX + bx;

						const FakeBlockData &blockB = ballsB.GetBlock(0, i);
						const FakeBlockData &blockF = ballsF.GetBlock(0, i);

						// firstly calculate result block and write it to temporary place, not to dst
						ResultBlock(TmpBlock, nBlkPitch,
							pPlanesB[0]->GetPointer(blockB.GetX() * nPel + ((blockB.GetMV().x * (256 - time256)) >> 8), blockB.GetY() * nPel + ((blockB.GetMV().y * (256 - time256)) >> 8)),
							pPlanesB[0]->GetPitch(),
							pPlanesF[0]->GetPointer(blockF.GetX() * nPel + ((blockF.GetMV().x * time256) >> 8), blockF.GetY() * nPel + ((blockF.GetMV().y * time256) >> 8)),
							pPlanesF[0]->GetPitch(),
							pRef[0] + xx * bytesPerSample, nRefPitches[0],
							pSrc[0] + xx * bytesPerSample, nSrcPitches[0],
							pMaskFullYB + xx, nPitchY,
							pMaskFullYF + xx, pMaskOccY + xx,
							nBlkSizeX, nBlkSizeY, time256, mode);
						d->OVERSLUMA(pDstTemp + xx * bytesPerSample * 2, dstTempPitch, TmpBlock, nBlkPitch, winOver, nBlkSizeX);
						if (nSuperModeYUV & UVPLANES) {
							ResultBlock(TmpBlock, nBlkPitch,
								pPlanesB[1]->GetPointer((blockB.GetX() * nPel + ((blockB.GetMV().x * (256 - time256)) >> 8)) / xRatioUV, (blockB.GetY() * nPel + ((blockB.GetMV().y * (256 - time256)) >> 8)) / yRatioUV),
								pPlanesB[1]->GetPitch(),
								pPlanesF[1]->GetPointer((blockF.GetX() * nPel + ((blockF.GetMV().x * time256) >> 8)) / xRatioUV, (blockF.GetY() * nPel + ((blockF.GetMV().y * time256) >> 8)) / yRatioUV),
								pPlanesF[1]->GetPitch(),
								pRef[1] + xxUV * bytesPerSample, nRefPitches[1],
								pSrc[1] + xxUV * bytesPerSample, nSrcPitches[1],
								pMaskFullUVB + xxUV, nPitchUV,
								pMaskFullUVF + xxUV, pMaskOccUV + xxUV,
								nBlkSizeX / xRatioUV, nBlkSizeY / yRatioUV, time256, mode);
							d->OVERSCHROMA(pDstTempU + xxUV * bytesPerSample * 2, dstTempPitchUV, TmpBlock, nBlkPitch, winOverUV, nBlkSizeX / xRatioUV);
							ResultBlock(TmpBlock, nBlkPitch,
								pPlanesB[2]->GetPointer((blockB.GetX() * nPel + ((blockB.GetMV().x * (256 - time256)) >> 8)) / xRatioUV, (blockB.GetY() * nPel + ((blockB.GetMV().y * (256 - time256)) >> 8)) / yRatioUV),
								pPlanesB[2]->GetPitch(),
								pPlanesF[2]->GetPointer((blockF.GetX() * nPel + ((blockF.GetMV().x * time256) >> 8)) / xRatioUV, (blockF.GetY() * nPel + ((blockF.GetMV().y * time256) >> 8)) / yRatioUV),
								pPlanesF[2]->GetPitch(),
								pRef[2] + xxUV * bytesPerSample, nRefPitches[2],
								pSrc[2] + xxUV * bytesPerSample, nSrcPitches[2],
								pMaskFullUVB + xxUV, nPitchUV,
								pMaskFullUVF + xxUV, pMaskOccUV + xxUV,
								nBlkSizeX / xRatioUV, nBlkSizeY / yRatioUV, time256, mode);
							d->OVERSCHROMA(pDstTempV + xxUV * bytesPerSample * 2, dstTempPitchUV, TmpBlock, nBlkPitch, winOverUV, nBlkSizeX / xRatioUV);
						}

						xx += (nBlkSizeX - nOverlapX);
						xxUV += (nBlkSizeX - nOverlapX) / xRatioUV;
					}

					pDstTemp += dstTempPitch * (nBlkSizeY - nOverlapY);
					pDstTempU += dstTempPitchUV * (nBlkSizeY - nOverlapY) / yRatioUV;
					pDstTempV += dstTempPitchUV * (nBlkSizeY - nOverlapY) / yRatioUV;
					pDst[0] += nDstPitches[0] * (nBlkSizeY - nOverlapY);
					pDst[1] += nDstPitches[1] * (nBlkSizeY - nOverlapY) / yRatioUV;
					pDst[2] += nDstPitches[2] * (nBlkSizeY - nOverlapY) / yRatioUV;
					pRef[0] += nRefPitches[0] * (nBlkSizeY - nOverlapY);
					pRef[1] += nRefPitches[1] * (nBlkSizeY - nOverlapY) / yRatioUV;
					pRef[2] += nRefPitches[2] * (nBlkSizeY - nOverlapY) / yRatioUV;
					pSrc[0] += nSrcPitches[0] * (nBlkSizeY - nOverlapY);
					pSrc[1] += nSrcPitches[1] * (nBlkSizeY - nOverlapY) / yRatioUV;
					pSrc[2] += nSrcPitches[2] * (nBlkSizeY - nOverlapY) / yRatioUV;
					pMaskFullYB += nPitchY * (nBlkSizeY - nOverlapY);
					pMaskFullUVB += nPitchUV * (nBlkSizeY - nOverlapY) / yRatioUV;
					pMaskFullYF += nPitchY * (nBlkSizeY - nOverlapY);
					pMaskFullUVF += nPitchUV * (nBlkSizeY - nOverlapY) / yRatioUV;
					pMaskOccY += nPitchY * (nBlkSizeY - nOverlapY);
					pMaskOccUV += nPitchUV * (nBlkSizeY - nOverlapY) / yRatioUV;
				}

				for (int i = 0; i < d->supervi->format->numPlanes; i++)
					pDst[i] = vsapi->getWritePtr(dst, i);

				d->ToPixels(pDst[0], nDstPitches[0], DstTemp, dstTempPitch, nWidth_B, nHeight_B);
				if (nSuperModeYUV & UVPLANES) {
					d->ToPixels(pDst[1], nDstPitches[1], DstTempU, dstTempPitchUV, nWidth_B / xRatioUV, nHeight_B / yRatioUV);
					d->ToPixels(pDst[2], nDstPitches[2], DstTempV, dstTempPitchUV, nWidth_B / xRatioUV, nHeight_B / yRatioUV);
				}

				delete[] TmpBlock;
				delete[] DstTemp;
				if (nSuperModeYUV & UVPLANES) {
					delete[] DstTempU;
					delete[] DstTempV;
				}
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
			const VSFrameRef *src = vsapi->getFrameFilter(VSMIN(nleft, d->oldvi->numFrames - 1), d->node, frameCtx);
			if (blend) {
				uint8_t *pDst[3];
				const uint8_t *pRef[3], *pSrc[3];
				int32_t nDstPitches[3], nRefPitches[3], nSrcPitches[3];
				const VSFrameRef *ref = vsapi->getFrameFilter(VSMIN(nright, d->oldvi->numFrames - 1), d->node, frameCtx);
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
	if (d->bleh->nOverlapX || d->bleh->nOverlapY) {
		delete d->OverWins;
		if (d->nSuperModeYUV & UVPLANES)
			delete d->OverWinsUV;
	}
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
	const int32_t xRatioUV = d->bleh->xRatioUV;
	const int32_t yRatioUV = d->bleh->yRatioUV;
	const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
	const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
	static OverlapsFunction overs[257][257];
	overs[2][2] = Overlaps_C<2, 2, double, float>;
	overs[2][4] = Overlaps_C<2, 4, double, float>;
	overs[4][2] = Overlaps_C<4, 2, double, float>;
	overs[4][4] = Overlaps_C<4, 4, double, float>;
	overs[4][8] = Overlaps_C<4, 8, double, float>;
	overs[8][1] = Overlaps_C<8, 1, double, float>;
	overs[8][2] = Overlaps_C<8, 2, double, float>;
	overs[8][4] = Overlaps_C<8, 4, double, float>;
	overs[8][8] = Overlaps_C<8, 8, double, float>;
	overs[8][16] = Overlaps_C<8, 16, double, float>;
	overs[16][1] = Overlaps_C<16, 1, double, float>;
	overs[16][2] = Overlaps_C<16, 2, double, float>;
	overs[16][4] = Overlaps_C<16, 4, double, float>;
	overs[16][8] = Overlaps_C<16, 8, double, float>;
	overs[16][16] = Overlaps_C<16, 16, double, float>;
	overs[16][32] = Overlaps_C<16, 32, double, float>;
	overs[32][8] = Overlaps_C<32, 8, double, float>;
	overs[32][16] = Overlaps_C<32, 16, double, float>;
	overs[32][32] = Overlaps_C<32, 32, double, float>;
	overs[32][64] = Overlaps_C<32, 64, double, float>;
	overs[64][16] = Overlaps_C<64, 16, double, float>;
	overs[64][32] = Overlaps_C<64, 32, double, float>;
	overs[64][64] = Overlaps_C<64, 64, double, float>;
	overs[64][128] = Overlaps_C<64, 128, double, float>;
	overs[128][32] = Overlaps_C<128, 32, double, float>;
	overs[128][64] = Overlaps_C<128, 64, double, float>;
	overs[128][128] = Overlaps_C<128, 128, double, float>;
	overs[128][256] = Overlaps_C<128, 256, double, float>;
	overs[256][64] = Overlaps_C<256, 64, double, float>;
	overs[256][128] = Overlaps_C<256, 128, double, float>;
	overs[256][256] = Overlaps_C<256, 256, double, float>;
	d->ToPixels = ToPixels<double, float>;
	d->OVERSLUMA = overs[nBlkSizeX][nBlkSizeY];
	d->OVERSCHROMA = overs[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
}

static void VS_CC mvblockfpsCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVBlockFPSData d;
	MVBlockFPSData *data;
	int err;
	d.num = vsapi->propGetInt(in, "num", 0, &err);
	if (err)
		d.num = 25;
	d.den = vsapi->propGetInt(in, "den", 0, &err);
	if (err)
		d.den = 1;
	d.mode = int64ToIntS(vsapi->propGetInt(in, "mode", 0, &err));
	if (err)
		d.mode = 3;
	d.ml = vsapi->propGetFloat(in, "ml", 0, &err);
	if (err)
		d.ml = 100.0;
	d.blend = !!vsapi->propGetInt(in, "blend", 0, &err);
	if (err)
		d.blend = 1;
	d.thscd1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.thscd1 = MV_DEFAULT_SCD1;
	d.thscd2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.thscd2 = MV_DEFAULT_SCD2;
	if (d.mode < 0 || d.mode > 8) {
		vsapi->setError(out, "BlockFPS: mode must be between 0 and 8 (inclusive).");
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
	d.oldvi = vsapi->getVideoInfo(d.node);
	d.vi = *d.oldvi;
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
	if (!isConstantFormat(&d.vi) || d.vi.format->bitsPerSample < 32 || d.vi.format->sampleType != stFloat) {
		vsapi->setError(out, "BlockFPS: input clip must be single precision fp, with constant dimensions.");
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
	d.upsizer = new SimpleResize<double>(d.nWidthP, d.nHeightP, d.nBlkXP, d.nBlkYP);
	if (d.nSuperModeYUV & UVPLANES)
		d.upsizerUV = new SimpleResize<double>(d.nWidthPUV, d.nHeightPUV, d.nBlkXP, d.nBlkYP);
	if (d.bleh->nOverlapX || d.bleh->nOverlapY) {
		d.OverWins = new OverlapWindows(d.bleh->nBlkSizeX, d.bleh->nBlkSizeY, d.bleh->nOverlapX, d.bleh->nOverlapY);
		if (d.nSuperModeYUV & UVPLANES)
			d.OverWinsUV = new OverlapWindows(d.bleh->nBlkSizeX / d.bleh->xRatioUV, d.bleh->nBlkSizeY / d.bleh->yRatioUV, d.bleh->nOverlapX / d.bleh->xRatioUV, d.bleh->nOverlapY / d.bleh->yRatioUV);
	}
	d.dstTempPitch = ((d.bleh->nWidth + 15) / 16) * 16 * d.vi.format->bytesPerSample * 2;
	d.dstTempPitchUV = (((d.bleh->nWidth / d.bleh->xRatioUV) + 15) / 16) * 16 * d.vi.format->bytesPerSample * 2;
	d.nBlkPitch = ((d.bleh->nBlkSizeX + 15) & (~15)) * d.vi.format->bytesPerSample;
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
		"ml:float:opt;"
		"blend:int:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvblockfpsCreate, 0, plugin);
}
