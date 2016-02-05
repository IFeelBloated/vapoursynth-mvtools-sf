#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "VapourSynth.h"
#include "VSHelper.h"
#include "MaskFun.h"
#include "MVFilter.h"
#include "MVInterface.h"
#include "SimpleResize.h"

typedef struct {
	VSNodeRef *node;
	const VSVideoInfo *vi;
	VSNodeRef *finest;
	VSNodeRef *super;
	VSNodeRef *mvbw;
	VSNodeRef *mvfw;
	float time;
	float ml;
	bool blend;
	float thscd1;
	float thscd2;
	MVClipDicks *mvClipB;
	MVClipDicks *mvClipF;
	MVFilter *bleh;
	int32_t nSuperHPad;
	int32_t nBlkXP;
	int32_t nBlkYP;
	int32_t nWidthP;
	int32_t nHeightP;
	int32_t nWidthPUV;
	int32_t nHeightPUV;
	int32_t nWidthUV;
	int32_t nHeightUV;
	int32_t nVPaddingUV;
	int32_t nHPaddingUV;
	int32_t VPitchY;
	int32_t VPitchUV;
	int32_t time256;
	int32_t *LUTVB;
	int32_t *LUTVF;
	SimpleResize *upsizer;
	SimpleResize *upsizerUV;
} MVFlowInterData;

static void VS_CC mvflowinterInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData *d = (MVFlowInterData *)* instanceData;
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvflowinterGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData *d = (MVFlowInterData *)* instanceData;

	if (activationReason == arInitial) {
		int32_t off = d->mvClipB->GetDeltaFrame(); // integer offset of reference frame

		if (n + off < d->vi->numFrames || !d->vi->numFrames) {
			vsapi->requestFrameFilter(n, d->mvfw, frameCtx);
			vsapi->requestFrameFilter(n + off, d->mvfw, frameCtx);

			vsapi->requestFrameFilter(n, d->mvbw, frameCtx);
			vsapi->requestFrameFilter(n + off, d->mvbw, frameCtx);

			vsapi->requestFrameFilter(n, d->finest, frameCtx);
			vsapi->requestFrameFilter(n + off, d->finest, frameCtx);
		}

		vsapi->requestFrameFilter(n, d->node, frameCtx);
		vsapi->requestFrameFilter(d->vi->numFrames ? VSMIN(n + off, d->vi->numFrames - 1) : n + off, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		uint8_t *pDst[3];
		const uint8_t *pRef[3], *pSrc[3];
		int32_t nDstPitches[3];
		int32_t nRefPitches[3];
		int32_t nSrcPitches[3];

		MVClipBalls ballsF(d->mvClipF, vsapi);
		MVClipBalls ballsB(d->mvClipB, vsapi);

		bool isUsableB = false;
		bool isUsableF = false;

		int32_t off = d->mvClipB->GetDeltaFrame(); // integer offset of reference frame

		if (n + off < d->vi->numFrames || !d->vi->numFrames) {
			const VSFrameRef *mvF = vsapi->getFrameFilter(n + off, d->mvfw, frameCtx);
			ballsF.Update(mvF);
			vsapi->freeFrame(mvF);
			isUsableF = ballsF.IsUsable();

			const VSFrameRef *mvB = vsapi->getFrameFilter(n, d->mvbw, frameCtx);
			ballsB.Update(mvB);
			vsapi->freeFrame(mvB);
			isUsableB = ballsB.IsUsable();
		}

		const int32_t nWidth = d->bleh->nWidth;
		const int32_t nHeight = d->bleh->nHeight;
		const int32_t nWidthUV = d->nWidthUV;
		const int32_t nHeightUV = d->nHeightUV;
		const int32_t time256 = d->time256;
		const bool blend = d->blend;

		int32_t bytesPerSample = d->vi->format->bytesPerSample;

		if (isUsableB && isUsableF)
		{
			const VSFrameRef *src = vsapi->getFrameFilter(n, d->finest, frameCtx);
			const VSFrameRef *ref = vsapi->getFrameFilter(n + off, d->finest, frameCtx); //  ref for  compensation
			VSFrameRef *dst = vsapi->newVideoFrame(d->vi->format, d->vi->width, d->vi->height, src, core);

			for (int32_t i = 0; i < d->vi->format->numPlanes; i++) {
				pDst[i] = vsapi->getWritePtr(dst, i);
				pRef[i] = vsapi->getReadPtr(ref, i);
				pSrc[i] = vsapi->getReadPtr(src, i);
				nDstPitches[i] = vsapi->getStride(dst, i);
				nRefPitches[i] = vsapi->getStride(ref, i);
				nSrcPitches[i] = vsapi->getStride(src, i);
			}

			const float ml = d->ml;
			const int32_t xRatioUV = d->bleh->xRatioUV;
			const int32_t yRatioUV = d->bleh->yRatioUV;
			const int32_t nBlkX = d->bleh->nBlkX;
			const int32_t nBlkY = d->bleh->nBlkY;
			const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
			const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
			const int32_t nOverlapX = d->bleh->nOverlapX;
			const int32_t nOverlapY = d->bleh->nOverlapY;
			const int32_t nVPadding = d->bleh->nVPadding;
			const int32_t nHPadding = d->bleh->nHPadding;
			const int32_t nVPaddingUV = d->nVPaddingUV;
			const int32_t nHPaddingUV = d->nHPaddingUV;
			const int32_t nPel = d->bleh->nPel;
			const int32_t VPitchY = d->VPitchY;
			const int32_t VPitchUV = d->VPitchUV;
			const int32_t nHeightP = d->nHeightP;
			const int32_t nHeightPUV = d->nHeightPUV;
			const int32_t nBlkXP = d->nBlkXP;
			const int32_t nBlkYP = d->nBlkYP;
			SimpleResize *upsizer = d->upsizer;
			SimpleResize *upsizerUV = d->upsizerUV;
			const int32_t *LUTVB = d->LUTVB;
			const int32_t *LUTVF = d->LUTVF;

			int32_t nOffsetY = nRefPitches[0] * nVPadding * nPel + nHPadding * bytesPerSample * nPel;
			int32_t nOffsetUV = nRefPitches[1] * nVPaddingUV * nPel + nHPaddingUV * bytesPerSample * nPel;


			uint8_t *VXFullYB = new uint8_t[nHeightP * VPitchY];
			uint8_t *VYFullYB = new uint8_t[nHeightP * VPitchY];
			uint8_t *VXFullYF = new uint8_t[nHeightP * VPitchY];
			uint8_t *VYFullYF = new uint8_t[nHeightP * VPitchY];
			uint8_t *VXSmallYB = new uint8_t[nBlkXP * nBlkYP];
			uint8_t *VYSmallYB = new uint8_t[nBlkXP * nBlkYP];
			uint8_t *VXSmallYF = new uint8_t[nBlkXP * nBlkYP];
			uint8_t *VYSmallYF = new uint8_t[nBlkXP * nBlkYP];
			uint8_t *MaskSmallB = new uint8_t[nBlkXP * nBlkYP];
			uint8_t *MaskFullYB = new uint8_t[nHeightP * VPitchY];
			uint8_t *MaskSmallF = new uint8_t[nBlkXP * nBlkYP];
			uint8_t *MaskFullYF = new uint8_t[nHeightP * VPitchY];
			uint8_t *VXFullUVB = nullptr;
			uint8_t *VYFullUVB = nullptr;
			uint8_t *VXFullUVF = nullptr;
			uint8_t *VYFullUVF = nullptr;
			uint8_t *VXSmallUVB = nullptr;
			uint8_t *VYSmallUVB = nullptr;
			uint8_t *VXSmallUVF = nullptr;
			uint8_t *VYSmallUVF = nullptr;
			uint8_t *MaskFullUVB = nullptr;
			uint8_t *MaskFullUVF = nullptr;


			// make  vector vx and vy small masks
			// 1. ATTENTION: vectors are assumed SHORT (|vx|, |vy| < 127) !
			// 2. they will be zeroed if not
			// 3. added 128 to all values
			MakeVectorSmallMasks(&ballsB, nBlkX, nBlkY, VXSmallYB, nBlkXP, VYSmallYB, nBlkXP);
			MakeVectorSmallMasks(&ballsF, nBlkX, nBlkY, VXSmallYF, nBlkXP, VYSmallYF, nBlkXP);
			if (nBlkXP > nBlkX) // fill right
			{
				for (int32_t j = 0; j<nBlkY; j++)
				{
					VXSmallYB[j*nBlkXP + nBlkX] = VSMIN(VXSmallYB[j*nBlkXP + nBlkX - 1], 128);
					VYSmallYB[j*nBlkXP + nBlkX] = VYSmallYB[j*nBlkXP + nBlkX - 1];
					VXSmallYF[j*nBlkXP + nBlkX] = VSMIN(VXSmallYF[j*nBlkXP + nBlkX - 1], 128);
					VYSmallYF[j*nBlkXP + nBlkX] = VYSmallYF[j*nBlkXP + nBlkX - 1];
				}
			}
			if (nBlkYP > nBlkY) // fill bottom
			{
				for (int32_t i = 0; i<nBlkXP; i++)
				{
					VXSmallYB[nBlkXP*nBlkY + i] = VXSmallYB[nBlkXP*(nBlkY - 1) + i];
					VYSmallYB[nBlkXP*nBlkY + i] = VSMIN(VYSmallYB[nBlkXP*(nBlkY - 1) + i], 128);
					VXSmallYF[nBlkXP*nBlkY + i] = VXSmallYF[nBlkXP*(nBlkY - 1) + i];
					VYSmallYF[nBlkXP*nBlkY + i] = VSMIN(VYSmallYF[nBlkXP*(nBlkY - 1) + i], 128);
				}
			}
			// analyze vectors field to detect occlusion
			//      double occNormB = (256-time256)/(256*ml);
			MakeVectorOcclusionMaskTime(&ballsB, nBlkX, nBlkY, ml, 1.0, nPel, MaskSmallB, nBlkXP, (256 - time256), nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
			//      double occNormF = time256/(256*ml);
			MakeVectorOcclusionMaskTime(&ballsF, nBlkX, nBlkY, ml, 1.0, nPel, MaskSmallF, nBlkXP, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
			if (nBlkXP > nBlkX) // fill right
			{
				for (int32_t j = 0; j<nBlkY; j++)
				{
					MaskSmallB[j*nBlkXP + nBlkX] = MaskSmallB[j*nBlkXP + nBlkX - 1];
					MaskSmallF[j*nBlkXP + nBlkX] = MaskSmallF[j*nBlkXP + nBlkX - 1];
				}
			}
			if (nBlkYP > nBlkY) // fill bottom
			{
				for (int32_t i = 0; i<nBlkXP; i++)
				{
					MaskSmallB[nBlkXP*nBlkY + i] = MaskSmallB[nBlkXP*(nBlkY - 1) + i];
					MaskSmallF[nBlkXP*nBlkY + i] = MaskSmallF[nBlkXP*(nBlkY - 1) + i];
				}
			}
			// upsize (bilinear interpolate) vector masks to fullframe size


			upsizer->Resize(VXFullYB, VPitchY, VXSmallYB, nBlkXP);
			upsizer->Resize(VYFullYB, VPitchY, VYSmallYB, nBlkXP);
			upsizer->Resize(VXFullYF, VPitchY, VXSmallYF, nBlkXP);
			upsizer->Resize(VYFullYF, VPitchY, VYSmallYF, nBlkXP);
			upsizer->Resize(MaskFullYB, VPitchY, MaskSmallB, nBlkXP);
			upsizer->Resize(MaskFullYF, VPitchY, MaskSmallF, nBlkXP);

			if (d->vi->format->colorFamily != cmGray) {
				VXFullUVB = new uint8_t[nHeightPUV * VPitchUV];
				VYFullUVB = new uint8_t[nHeightPUV * VPitchUV];
				VXFullUVF = new uint8_t[nHeightPUV * VPitchUV];
				VYFullUVF = new uint8_t[nHeightPUV * VPitchUV];
				VXSmallUVB = new uint8_t[nBlkXP * nBlkYP];
				VYSmallUVB = new uint8_t[nBlkXP * nBlkYP];
				VXSmallUVF = new uint8_t[nBlkXP * nBlkYP];
				VYSmallUVF = new uint8_t[nBlkXP * nBlkYP];
				MaskFullUVB = new uint8_t[nHeightPUV * VPitchUV];
				MaskFullUVF = new uint8_t[nHeightPUV * VPitchUV];

				VectorSmallMaskYToHalfUV(VXSmallYB, nBlkXP, nBlkYP, VXSmallUVB, xRatioUV);
				VectorSmallMaskYToHalfUV(VYSmallYB, nBlkXP, nBlkYP, VYSmallUVB, yRatioUV);
				VectorSmallMaskYToHalfUV(VXSmallYF, nBlkXP, nBlkYP, VXSmallUVF, xRatioUV);
				VectorSmallMaskYToHalfUV(VYSmallYF, nBlkXP, nBlkYP, VYSmallUVF, yRatioUV);

				upsizerUV->Resize(VXFullUVB, VPitchUV, VXSmallUVB, nBlkXP);
				upsizerUV->Resize(VYFullUVB, VPitchUV, VYSmallUVB, nBlkXP);
				upsizerUV->Resize(VXFullUVF, VPitchUV, VXSmallUVF, nBlkXP);
				upsizerUV->Resize(VYFullUVF, VPitchUV, VYSmallUVF, nBlkXP);
				upsizerUV->Resize(MaskFullUVB, VPitchUV, MaskSmallB, nBlkXP);
				upsizerUV->Resize(MaskFullUVF, VPitchUV, MaskSmallF, nBlkXP);
			}


			const VSFrameRef *mvFF = vsapi->getFrameFilter(n, d->mvfw, frameCtx);
			ballsF.Update(mvFF);
			vsapi->freeFrame(mvFF);

			const VSFrameRef *mvBB = vsapi->getFrameFilter(n + off, d->mvbw, frameCtx);
			ballsB.Update(mvBB);
			vsapi->freeFrame(mvBB);


			if (ballsB.IsUsable() && ballsF.IsUsable())
			{
				uint8_t *VXFullYBB = new uint8_t[nHeightP * VPitchY];
				uint8_t *VYFullYBB = new uint8_t[nHeightP * VPitchY];
				uint8_t *VXFullYFF = new uint8_t[nHeightP * VPitchY];
				uint8_t *VYFullYFF = new uint8_t[nHeightP * VPitchY];
				uint8_t *VXSmallYBB = new uint8_t[nBlkXP * nBlkYP];
				uint8_t *VYSmallYBB = new uint8_t[nBlkXP * nBlkYP];
				uint8_t *VXSmallYFF = new uint8_t[nBlkXP * nBlkYP];
				uint8_t *VYSmallYFF = new uint8_t[nBlkXP * nBlkYP];

				// get vector mask from extra frames
				MakeVectorSmallMasks(&ballsB, nBlkX, nBlkY, VXSmallYBB, nBlkXP, VYSmallYBB, nBlkXP);
				MakeVectorSmallMasks(&ballsF, nBlkX, nBlkY, VXSmallYFF, nBlkXP, VYSmallYFF, nBlkXP);
				if (nBlkXP > nBlkX) // fill right
				{
					for (int32_t j = 0; j<nBlkY; j++)
					{
						VXSmallYBB[j*nBlkXP + nBlkX] = VSMIN(VXSmallYBB[j*nBlkXP + nBlkX - 1], 128);
						VYSmallYBB[j*nBlkXP + nBlkX] = VYSmallYBB[j*nBlkXP + nBlkX - 1];
						VXSmallYFF[j*nBlkXP + nBlkX] = VSMIN(VXSmallYFF[j*nBlkXP + nBlkX - 1], 128);
						VYSmallYFF[j*nBlkXP + nBlkX] = VYSmallYFF[j*nBlkXP + nBlkX - 1];
					}
				}
				if (nBlkYP > nBlkY) // fill bottom
				{
					for (int32_t i = 0; i<nBlkXP; i++)
					{
						VXSmallYBB[nBlkXP*nBlkY + i] = VXSmallYBB[nBlkXP*(nBlkY - 1) + i];
						VYSmallYBB[nBlkXP*nBlkY + i] = VSMIN(VYSmallYBB[nBlkXP*(nBlkY - 1) + i], 128);
						VXSmallYFF[nBlkXP*nBlkY + i] = VXSmallYFF[nBlkXP*(nBlkY - 1) + i];
						VYSmallYFF[nBlkXP*nBlkY + i] = VSMIN(VYSmallYFF[nBlkXP*(nBlkY - 1) + i], 128);
					}
				}

				// upsize vectors to full frame
				upsizer->Resize(VXFullYBB, VPitchY, VXSmallYBB, nBlkXP);
				upsizer->Resize(VYFullYBB, VPitchY, VYSmallYBB, nBlkXP);
				upsizer->Resize(VXFullYFF, VPitchY, VXSmallYFF, nBlkXP);
				upsizer->Resize(VYFullYFF, VPitchY, VYSmallYFF, nBlkXP);

				FlowInterExtra(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, pSrc[0] + nOffsetY, nRefPitches[0],
					VXFullYB, VXFullYF, VYFullYB, VYFullYF, MaskFullYB, MaskFullYF, VPitchY,
					nWidth, nHeight, time256, nPel, LUTVB, LUTVF, VXFullYBB, VXFullYFF, VYFullYBB, VYFullYFF);

				if (d->vi->format->colorFamily != cmGray) {
					uint8_t *VXFullUVFF = new uint8_t[nHeightPUV * VPitchUV];
					uint8_t *VXFullUVBB = new uint8_t[nHeightPUV * VPitchUV];
					uint8_t *VYFullUVBB = new uint8_t[nHeightPUV * VPitchUV];
					uint8_t *VYFullUVFF = new uint8_t[nHeightPUV * VPitchUV];
					uint8_t *VXSmallUVBB = new uint8_t[nBlkXP * nBlkYP];
					uint8_t *VYSmallUVBB = new uint8_t[nBlkXP * nBlkYP];
					uint8_t *VXSmallUVFF = new uint8_t[nBlkXP * nBlkYP];
					uint8_t *VYSmallUVFF = new uint8_t[nBlkXP * nBlkYP];

					VectorSmallMaskYToHalfUV(VXSmallYBB, nBlkXP, nBlkYP, VXSmallUVBB, xRatioUV);
					VectorSmallMaskYToHalfUV(VYSmallYBB, nBlkXP, nBlkYP, VYSmallUVBB, yRatioUV);
					VectorSmallMaskYToHalfUV(VXSmallYFF, nBlkXP, nBlkYP, VXSmallUVFF, xRatioUV);
					VectorSmallMaskYToHalfUV(VYSmallYFF, nBlkXP, nBlkYP, VYSmallUVFF, yRatioUV);

					upsizerUV->Resize(VXFullUVBB, VPitchUV, VXSmallUVBB, nBlkXP);
					upsizerUV->Resize(VYFullUVBB, VPitchUV, VYSmallUVBB, nBlkXP);
					upsizerUV->Resize(VXFullUVFF, VPitchUV, VXSmallUVFF, nBlkXP);
					upsizerUV->Resize(VYFullUVFF, VPitchUV, VYSmallUVFF, nBlkXP);

					FlowInterExtra(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, pSrc[1] + nOffsetUV, nRefPitches[1],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF, VXFullUVBB, VXFullUVFF, VYFullUVBB, VYFullUVFF);
					FlowInterExtra(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, pSrc[2] + nOffsetUV, nRefPitches[2],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF, VXFullUVBB, VXFullUVFF, VYFullUVBB, VYFullUVFF);

					delete[] VXFullUVBB;
					delete[] VYFullUVBB;
					delete[] VXSmallUVBB;
					delete[] VYSmallUVBB;
					delete[] VXFullUVFF;
					delete[] VYFullUVFF;
					delete[] VXSmallUVFF;
					delete[] VYSmallUVFF;
				}

				delete[] VXFullYBB;
				delete[] VYFullYBB;
				delete[] VXSmallYBB;
				delete[] VYSmallYBB;
				delete[] VXFullYFF;
				delete[] VYFullYFF;
				delete[] VXSmallYFF;
				delete[] VYSmallYFF;
			}
			else // bad extra frames, use old method without extra frames
			{
				FlowInter(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, pSrc[0] + nOffsetY, nRefPitches[0],
					VXFullYB, VXFullYF, VYFullYB, VYFullYF, MaskFullYB, MaskFullYF, VPitchY,
					nWidth, nHeight, time256, nPel, LUTVB, LUTVF);
				if (d->vi->format->colorFamily != cmGray) {
					FlowInter(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, pSrc[1] + nOffsetUV, nRefPitches[1],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF);
					FlowInter(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, pSrc[2] + nOffsetUV, nRefPitches[2],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF);
				}
			}


			delete[] VXFullYB;
			delete[] VYFullYB;
			delete[] VXSmallYB;
			delete[] VYSmallYB;
			delete[] VXFullYF;
			delete[] VYFullYF;
			delete[] VXSmallYF;
			delete[] VYSmallYF;
			delete[] MaskSmallB;
			delete[] MaskFullYB;
			delete[] MaskSmallF;
			delete[] MaskFullYF;

			if (d->vi->format->colorFamily != cmGray) {
				delete[] VXFullUVB;
				delete[] VYFullUVB;
				delete[] VXSmallUVB;
				delete[] VYSmallUVB;
				delete[] VXFullUVF;
				delete[] VYFullUVF;
				delete[] VXSmallUVF;
				delete[] VYSmallUVF;
				delete[] MaskFullUVB;
				delete[] MaskFullUVF;
			}

			vsapi->freeFrame(src);
			vsapi->freeFrame(ref);

			return dst;
		}
		else // not usable
		{
			// poor estimation

			const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);

			if (blend) //let's blend src with ref frames like ConvertFPS
			{
				const VSFrameRef *ref = vsapi->getFrameFilter(d->vi->numFrames ? VSMIN(n + off, d->vi->numFrames - 1) : n + off, d->node, frameCtx);

				VSFrameRef *dst = vsapi->newVideoFrame(d->vi->format, d->vi->width, d->vi->height, src, core);

				for (int32_t i = 0; i < d->vi->format->numPlanes; i++) {
					pDst[i] = vsapi->getWritePtr(dst, i);
					pRef[i] = vsapi->getReadPtr(ref, i);
					pSrc[i] = vsapi->getReadPtr(src, i);
					nDstPitches[i] = vsapi->getStride(dst, i);
					nRefPitches[i] = vsapi->getStride(ref, i);
					nSrcPitches[i] = vsapi->getStride(src, i);
				}

				// blend with time weight
				Blend(pDst[0], pSrc[0], pRef[0], nHeight, nWidth, nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
				if (d->vi->format->colorFamily != cmGray) {
					Blend(pDst[1], pSrc[1], pRef[1], nHeightUV, nWidthUV, nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
					Blend(pDst[2], pSrc[2], pRef[2], nHeightUV, nWidthUV, nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);
				}

				vsapi->freeFrame(src);
				vsapi->freeFrame(ref);

				return dst;
			}
			else // no blend
			{
				return src; // like ChangeFPS
			}

		}
	}

	return 0;
}


static void VS_CC mvflowinterFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData *d = (MVFlowInterData *)instanceData;

	delete d->mvClipB;
	delete d->mvClipF;

	delete d->bleh;

	delete d->upsizer;
	if (d->vi->format->colorFamily != cmGray)
		delete d->upsizerUV;

	delete d->LUTVB;
	delete d->LUTVF;

	vsapi->freeNode(d->finest);
	vsapi->freeNode(d->super);
	vsapi->freeNode(d->mvfw);
	vsapi->freeNode(d->mvbw);
	vsapi->freeNode(d->node);
	free(d);
}


static void VS_CC mvflowinterCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData d;
	MVFlowInterData *data;

	int err;

	d.time = (float)vsapi->propGetFloat(in, "time", 0, &err);
	if (err)
		d.time = 50.0f;

	d.ml = (float)vsapi->propGetFloat(in, "ml", 0, &err);
	if (err)
		d.ml = 100.0f;

	d.blend = !!vsapi->propGetInt(in, "blend", 0, &err);
	if (err)
		d.blend = 1;

	d.thscd1 = static_cast<float>(vsapi->propGetFloat(in, "thscd1", 0, &err));
	if (err)
		d.thscd1 = MV_DEFAULT_SCD1;

	d.thscd2 = static_cast<float>(vsapi->propGetFloat(in, "thscd2", 0, &err));
	if (err)
		d.thscd2 = MV_DEFAULT_SCD2;


	if (d.time < 0.0f || d.time > 100.0f) {
		vsapi->setError(out, "FlowInter: time must be between 0 and 100 % (inclusive).");
		return;
	}

	if (d.ml <= 0.0f) {
		vsapi->setError(out, "FlowInter: ml must be greater than 0.");
		return;
	}

	d.time256 = (int32_t)(d.time * 256.0f / 100.0f);


	d.super = vsapi->propGetNode(in, "super", 0, nullptr);

	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("FlowInter: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.super);
		return;
	}
	const VSMap *props = vsapi->getFramePropsRO(evil);
	int32_t evil_err[2];
	int32_t nHeightS = int64ToIntS(vsapi->propGetInt(props, "Super_height", 0, &evil_err[0]));
	d.nSuperHPad = int64ToIntS(vsapi->propGetInt(props, "Super_hpad", 0, &evil_err[1]));
	vsapi->freeFrame(evil);

	for (int32_t i = 0; i < 2; i++)
		if (evil_err[i]) {
			vsapi->setError(out, "FlowInter: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.super);
			return;
		}


	d.mvbw = vsapi->propGetNode(in, "mvbw", 0, nullptr);
	d.mvfw = vsapi->propGetNode(in, "mvfw", 0, nullptr);

	// XXX Fuck all this trying.
	try {
		d.mvClipB = new MVClipDicks(d.mvbw, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("FlowInter: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvbw);
		vsapi->freeNode(d.mvfw);
		return;
	}

	try {
		d.mvClipF = new MVClipDicks(d.mvfw, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("FlowInter: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		return;
	}

	// XXX Alternatively, use both clips' delta as offsets in GetFrame.
	if (d.mvClipF->GetDeltaFrame() != d.mvClipB->GetDeltaFrame()) {
		vsapi->setError(out, "FlowInter: mvbw and mvfw must be generated with the same delta.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	if (!d.mvClipB->IsBackward() || d.mvClipF->IsBackward()) {
		if (!d.mvClipB->IsBackward())
			vsapi->setError(out, "FlowInter: mvbw must be generated with isb=True.");
		else
			vsapi->setError(out, "FlowInter: mvfw must be generated with isb=False.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	try {
		d.bleh = new MVFilter(d.mvfw, "FlowInter", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("FlowInter: ").append(e.what()).c_str());
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
		vsapi->setError(out, std::string("FlowInter: ").append(e.what()).c_str());
		delete d.bleh;
		delete d.mvClipB;
		delete d.mvClipF;
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		return;
	}
	if (d.bleh->nPel == 1)
		d.finest = vsapi->cloneNodeRef(d.super); // v2.0.9.1
	else {
		VSPlugin *mvtoolsPlugin = vsapi->getPluginById("com.nodame.mvsf", core);
		VSPlugin *stdPlugin = vsapi->getPluginById("com.vapoursynth.std", core);
		VSMap *args = vsapi->createMap();
		vsapi->propSetNode(args, "super", d.super, paReplace);
		VSMap *ret = vsapi->invoke(mvtoolsPlugin, "Finest", args);
		if (vsapi->getError(ret)) {
			vsapi->setError(out, std::string("FlowInter: ").append(vsapi->getError(ret)).c_str());
			delete d.bleh;
			delete d.mvClipB;
			delete d.mvClipF;
			vsapi->freeNode(d.super);
			vsapi->freeNode(d.mvfw);
			vsapi->freeNode(d.mvbw);
			vsapi->freeMap(args);
			vsapi->freeMap(ret);
			return;
		}
		d.finest = vsapi->propGetNode(ret, "clip", 0, nullptr);
		vsapi->freeMap(ret);
		vsapi->clearMap(args);
		vsapi->propSetNode(args, "clip", d.finest, paReplace);
		vsapi->freeNode(d.finest);
		ret = vsapi->invoke(stdPlugin, "Cache", args);
		vsapi->freeMap(args);
		if (vsapi->getError(ret)) {
			vsapi->setError(out, std::string("FlowInter: ").append(vsapi->getError(ret)).c_str());
			delete d.bleh;
			delete d.mvClipB;
			delete d.mvClipF;
			vsapi->freeNode(d.super);
			vsapi->freeNode(d.mvfw);
			vsapi->freeNode(d.mvbw);
			vsapi->freeMap(ret);
			return;
		}
		d.finest = vsapi->propGetNode(ret, "clip", 0, nullptr);
		vsapi->freeMap(ret);
	}
	d.node = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = vsapi->getVideoInfo(d.node);
	const VSVideoInfo *supervi = vsapi->getVideoInfo(d.super);
	int32_t nSuperWidth = supervi->width;
	if (d.bleh->nHeight != nHeightS || d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2) {
		vsapi->setError(out, "FlowInter: wrong source or super clip frame size.");
		vsapi->freeNode(d.finest);
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
	d.nHPaddingUV = d.bleh->nHPadding / d.bleh->xRatioUV;
	d.nVPaddingUV = d.bleh->nVPadding / d.bleh->yRatioUV;
	d.VPitchY = (d.nWidthP + 15) & (~15);
	d.VPitchUV = (d.nWidthPUV + 15) & (~15);
	d.upsizer = new SimpleResize(d.nWidthP, d.nHeightP, d.nBlkXP, d.nBlkYP);
	if (d.vi->format->colorFamily != cmGray)
		d.upsizerUV = new SimpleResize(d.nWidthPUV, d.nHeightPUV, d.nBlkXP, d.nBlkYP);
	d.LUTVB = new int32_t[256];
	d.LUTVF = new int32_t[256];
	Create_LUTV(d.time256, d.LUTVB, d.LUTVF);
	data = (MVFlowInterData *)malloc(sizeof(d));
	*data = d;
	vsapi->createFilter(in, out, "FlowInter", mvflowinterInit, mvflowinterGetFrame, mvflowinterFree, fmParallel, 0, data, core);
}

void mvflowinterRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("FlowInter",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"time:float:opt;"
		"ml:float:opt;"
		"blend:int:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvflowinterCreate, 0, plugin);
}
