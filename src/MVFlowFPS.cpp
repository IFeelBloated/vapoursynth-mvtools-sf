#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "VapourSynth.h"
#include "VSHelper.h"
#include "CommonFunctions.h"
#include "MaskFun.h"
#include "MVFilter.h"
#include "SimpleResize.h"

struct MVFlowFPSData {
	VSNodeRef *node;
	VSVideoInfo vi;
	VSNodeRef *finest;
	VSNodeRef *super;
	VSNodeRef *mvbw;
	VSNodeRef *mvfw;
	int64_t num, den;
	int32_t maskmode;
	double ml;
	bool blend;
	double thscd1, thscd2;
	MVClipDicks *mvClipB;
	MVClipDicks *mvClipF;
	MVFilter *bleh;
	int32_t nSuperHPad;
	int32_t nWidthUV;
	int32_t nHeightUV;
	int32_t nVPaddingUV;
	int32_t nHPaddingUV;
	int32_t VPitchY;
	int32_t VPitchUV;
	int32_t nWidthP;
	int32_t nHeightP;
	int32_t nWidthPUV;
	int32_t nHeightPUV;
	int32_t nBlkXP;
	int32_t nBlkYP;
	int32_t *LUTVB;
	int32_t *LUTVF;
	uint8_t *VXFullYB;
	uint8_t *VXFullUVB;
	uint8_t *VYFullYB;
	uint8_t *VYFullUVB;
	uint8_t *VXSmallYB;
	uint8_t *VYSmallYB;
	uint8_t *VXSmallUVB;
	uint8_t *VYSmallUVB;
	uint8_t *VXFullYF;
	uint8_t *VXFullUVF;
	uint8_t *VYFullYF;
	uint8_t *VYFullUVF;
	uint8_t *VXSmallYF;
	uint8_t *VYSmallYF;
	uint8_t *VXSmallUVF;
	uint8_t *VYSmallUVF;
	uint8_t *VXFullYBB;
	uint8_t *VXFullUVBB;
	uint8_t *VYFullYBB;
	uint8_t *VYFullUVBB;
	uint8_t *VXSmallYBB;
	uint8_t *VYSmallYBB;
	uint8_t *VXSmallUVBB;
	uint8_t *VYSmallUVBB;
	uint8_t *VXFullYFF;
	uint8_t *VXFullUVFF;
	uint8_t *VYFullYFF;
	uint8_t *VYFullUVFF;
	uint8_t *VXSmallYFF;
	uint8_t *VYSmallYFF;
	uint8_t *VXSmallUVFF;
	uint8_t *VYSmallUVFF;
	uint8_t *MaskSmallB;
	uint8_t *MaskFullYB;
	uint8_t *MaskFullUVB;
	uint8_t *MaskSmallF;
	uint8_t *MaskFullYF;
	uint8_t *MaskFullUVF;
	SimpleResize *upsizer;
	SimpleResize *upsizerUV;
	int64_t fa, fb;
	int32_t nleftLast, nrightLast;
};

static void VS_CC mvflowfpsInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVFlowFPSData *d = reinterpret_cast<MVFlowFPSData *>(*instanceData);
	vsapi->setVideoInfo(&d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvflowfpsGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVFlowFPSData *d = reinterpret_cast<MVFlowFPSData *>(*instanceData);

	if (activationReason == arInitial) {
		int32_t off = d->mvClipB->GetDeltaFrame(); // integer offset of reference frame

		int32_t nleft = (int32_t)(n * d->fa / d->fb);
		int32_t nright = nleft + off;

		int32_t time256 = (int32_t)((double(n) * double(d->fa) / double(d->fb) - nleft) * 256 + 0.5);
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

		if ((nleft < d->vi.numFrames && nright < d->vi.numFrames) || !d->vi.numFrames) { // for the good estimation case
			if (d->maskmode == 2)
				vsapi->requestFrameFilter(nleft, d->mvfw, frameCtx); // requests nleft - off, nleft
			vsapi->requestFrameFilter(nright, d->mvfw, frameCtx); // requests nleft, nleft + off
			vsapi->requestFrameFilter(nleft, d->mvbw, frameCtx); // requests nleft, nleft + off
			if (d->maskmode == 2)
				vsapi->requestFrameFilter(nright, d->mvbw, frameCtx); // requests nleft + off, nleft + off + off

			vsapi->requestFrameFilter(nleft, d->finest, frameCtx);
			vsapi->requestFrameFilter(nright, d->finest, frameCtx);
		}

		vsapi->requestFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx);

		if (d->blend)
			vsapi->requestFrameFilter(d->vi.numFrames ? VSMIN(nright, d->vi.numFrames - 1) : nright, d->node, frameCtx);

	}
	else if (activationReason == arAllFramesReady) {
		int32_t nleft = (int32_t)(n * d->fa / d->fb);
		// intermediate product may be very large! Now I know how to multiply int64
		int32_t time256 = (int32_t)((double(n)*double(d->fa) / double(d->fb) - nleft) * 256 + 0.5);

		int32_t off = d->mvClipB->GetDeltaFrame(); // integer offset of reference frame
												   // usually off must be = 1
		if (off > 1)
			time256 = time256 / off;

		int32_t nright = nleft + off;

		if (time256 == 0) {
			return vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx); // simply left
		}
		else if (time256 == 256) {
			return vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nright, d->vi.numFrames - 1) : nright, d->node, frameCtx); // simply right
		}

		MVClipBalls ballsF(d->mvClipF, vsapi);
		MVClipBalls ballsB(d->mvClipB, vsapi);

		bool isUsableF = false;
		bool isUsableB = false;

		if ((nleft < d->vi.numFrames && nright < d->vi.numFrames) || !d->vi.numFrames) {
			const VSFrameRef *mvF = vsapi->getFrameFilter(nright, d->mvfw, frameCtx);
			ballsF.Update(mvF);// forward from current to next
			isUsableF = ballsF.IsUsable();
			vsapi->freeFrame(mvF);

			const VSFrameRef *mvB = vsapi->getFrameFilter(nleft, d->mvbw, frameCtx);
			ballsB.Update(mvB);// backward from next to current
			isUsableB = ballsB.IsUsable();
			vsapi->freeFrame(mvB);
		}

		const int32_t nWidth = d->bleh->nWidth;
		const int32_t nHeight = d->bleh->nHeight;
		const int32_t nWidthUV = d->nWidthUV;
		const int32_t nHeightUV = d->nHeightUV;
		const int32_t maskmode = d->maskmode;
		const bool blend = d->blend;
		const double ml = d->ml;
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
		const int32_t nBlkXP = d->nBlkXP;
		const int32_t nBlkYP = d->nBlkYP;
		SimpleResize *upsizer = d->upsizer;
		SimpleResize *upsizerUV = d->upsizerUV;
		int32_t *LUTVB = d->LUTVB;
		int32_t *LUTVF = d->LUTVF;

		uint8_t *VXFullYB = d->VXFullYB;
		uint8_t *VXFullUVB = d->VXFullUVB;
		uint8_t *VYFullYB = d->VYFullYB;
		uint8_t *VYFullUVB = d->VYFullUVB;
		uint8_t *VXSmallYB = d->VXSmallYB;
		uint8_t *VYSmallYB = d->VYSmallYB;
		uint8_t *VXSmallUVB = d->VXSmallUVB;
		uint8_t *VYSmallUVB = d->VYSmallUVB;
		uint8_t *VXFullYF = d->VXFullYF;
		uint8_t *VXFullUVF = d->VXFullUVF;
		uint8_t *VYFullYF = d->VYFullYF;
		uint8_t *VYFullUVF = d->VYFullUVF;
		uint8_t *VXSmallYF = d->VXSmallYF;
		uint8_t *VYSmallYF = d->VYSmallYF;
		uint8_t *VXSmallUVF = d->VXSmallUVF;
		uint8_t *VYSmallUVF = d->VYSmallUVF;

		uint8_t *VXFullYBB = d->VXFullYBB;
		uint8_t *VXFullUVBB = d->VXFullUVBB;
		uint8_t *VYFullYBB = d->VYFullYBB;
		uint8_t *VYFullUVBB = d->VYFullUVBB;
		uint8_t *VXSmallYBB = d->VXSmallYBB;
		uint8_t *VYSmallYBB = d->VYSmallYBB;
		uint8_t *VXSmallUVBB = d->VXSmallUVBB;
		uint8_t *VYSmallUVBB = d->VYSmallUVBB;
		uint8_t *VXFullYFF = d->VXFullYFF;
		uint8_t *VXFullUVFF = d->VXFullUVFF;
		uint8_t *VYFullYFF = d->VYFullYFF;
		uint8_t *VYFullUVFF = d->VYFullUVFF;
		uint8_t *VXSmallYFF = d->VXSmallYFF;
		uint8_t *VYSmallYFF = d->VYSmallYFF;
		uint8_t *VXSmallUVFF = d->VXSmallUVFF;
		uint8_t *VYSmallUVFF = d->VYSmallUVFF;

		uint8_t *MaskSmallB = d->MaskSmallB;
		uint8_t *MaskFullYB = d->MaskFullYB;
		uint8_t *MaskFullUVB = d->MaskFullUVB;
		uint8_t *MaskSmallF = d->MaskSmallF;
		uint8_t *MaskFullYF = d->MaskFullYF;
		uint8_t *MaskFullUVF = d->MaskFullUVF;

		int32_t bytesPerSample = d->vi.format->bytesPerSample;

		if (isUsableB && isUsableF) {
			uint8_t *pDst[3];
			const uint8_t *pRef[3], *pSrc[3];
			int32_t nDstPitches[3], nRefPitches[3];

			// If both are usable, that means both nleft and nright are less than numFrames, or that we don't have numFrames. Thus there is no need to check nleft and nright here.
			const VSFrameRef *src = vsapi->getFrameFilter(nleft, d->finest, frameCtx);
			const VSFrameRef *ref = vsapi->getFrameFilter(nright, d->finest, frameCtx);//  right frame for  compensation
			VSFrameRef *dst = vsapi->newVideoFrame(d->vi.format, d->vi.width, d->vi.height, src, core);

			Create_LUTV(time256, LUTVB, LUTVF); // lookup table

			for (int32_t i = 0; i < d->vi.format->numPlanes; i++) {
				pDst[i] = vsapi->getWritePtr(dst, i);
				pRef[i] = vsapi->getReadPtr(ref, i);
				pSrc[i] = vsapi->getReadPtr(src, i);
				nDstPitches[i] = vsapi->getStride(dst, i);
				nRefPitches[i] = vsapi->getStride(ref, i);
			}

			int32_t nOffsetY = nRefPitches[0] * nVPadding * nPel + nHPadding * bytesPerSample * nPel;
			int32_t nOffsetUV = nRefPitches[1] * nVPaddingUV * nPel + nHPaddingUV * bytesPerSample * nPel;

			if (nright != d->nrightLast) {
				// make  vector vx and vy small masks
				// 1. ATTENTION: vectors are assumed SHORT (|vx|, |vy| < 127) !
				// 2. they will be zeroed if not
				// 3. added 128 to all values
				MakeVectorSmallMasks(&ballsB, nBlkX, nBlkY, VXSmallYB, nBlkXP, VYSmallYB, nBlkXP);
				if (nBlkXP > nBlkX) {// fill right
					for (int32_t j = 0; j<nBlkY; j++) {
						VXSmallYB[j*nBlkXP + nBlkX] = VSMIN(VXSmallYB[j*nBlkXP + nBlkX - 1], 128);
						VYSmallYB[j*nBlkXP + nBlkX] = VYSmallYB[j*nBlkXP + nBlkX - 1];
					}
				}
				if (nBlkYP > nBlkY) {// fill bottom
					for (int32_t i = 0; i<nBlkXP; i++) {
						VXSmallYB[nBlkXP*nBlkY + i] = VXSmallYB[nBlkXP*(nBlkY - 1) + i];
						VYSmallYB[nBlkXP*nBlkY + i] = VSMIN(VYSmallYB[nBlkXP*(nBlkY - 1) + i], 128);
					}
				}

				upsizer->Resize(VXFullYB, VPitchY, VXSmallYB, nBlkXP);
				upsizer->Resize(VYFullYB, VPitchY, VYSmallYB, nBlkXP);

				if (d->vi.format->colorFamily != cmGray) {
					VectorSmallMaskYToHalfUV(VXSmallYB, nBlkXP, nBlkYP, VXSmallUVB, xRatioUV);
					VectorSmallMaskYToHalfUV(VYSmallYB, nBlkXP, nBlkYP, VYSmallUVB, yRatioUV);

					upsizerUV->Resize(VXFullUVB, VPitchUV, VXSmallUVB, nBlkXP);
					upsizerUV->Resize(VYFullUVB, VPitchUV, VYSmallUVB, nBlkXP);
				}
			}
			// analyze vectors field to detect occlusion
			//        double occNormB = (256-time256)/(256*ml);
			//        MakeVectorOcclusionMask(mvClipB, nBlkX, nBlkY, occNormB, 1.0, nPel, MaskSmallB, nBlkXP);
			MakeVectorOcclusionMaskTime(&ballsB, nBlkX, nBlkY, ml, 1.0, nPel, MaskSmallB, nBlkXP, (256 - time256), nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
			if (nBlkXP > nBlkX) // fill right
				for (int32_t j = 0; j<nBlkY; j++)
					MaskSmallB[j*nBlkXP + nBlkX] = MaskSmallB[j*nBlkXP + nBlkX - 1];
			if (nBlkYP > nBlkY) // fill bottom
				for (int32_t i = 0; i<nBlkXP; i++)
					MaskSmallB[nBlkXP*nBlkY + i] = MaskSmallB[nBlkXP*(nBlkY - 1) + i];

			upsizer->Resize(MaskFullYB, VPitchY, MaskSmallB, nBlkXP);
			if (d->vi.format->colorFamily != cmGray)
				upsizerUV->Resize(MaskFullUVB, VPitchUV, MaskSmallB, nBlkXP);

			d->nrightLast = nright;

			if (nleft != d->nleftLast) {
				// make  vector vx and vy small masks
				// 1. ATTENTION: vectors are assumed SHORT (|vx|, |vy| < 127) !
				// 2. they will be zeroed if not
				// 3. added 128 to all values
				MakeVectorSmallMasks(&ballsF, nBlkX, nBlkY, VXSmallYF, nBlkXP, VYSmallYF, nBlkXP);
				if (nBlkXP > nBlkX) {// fill right
					for (int32_t j = 0; j<nBlkY; j++) {
						VXSmallYF[j*nBlkXP + nBlkX] = VSMIN(VXSmallYF[j*nBlkXP + nBlkX - 1], 128);
						VYSmallYF[j*nBlkXP + nBlkX] = VYSmallYF[j*nBlkXP + nBlkX - 1];
					}
				}
				if (nBlkYP > nBlkY) {// fill bottom
					for (int32_t i = 0; i<nBlkXP; i++) {
						VXSmallYF[nBlkXP*nBlkY + i] = VXSmallYF[nBlkXP*(nBlkY - 1) + i];
						VYSmallYF[nBlkXP*nBlkY + i] = VSMIN(VYSmallYF[nBlkXP*(nBlkY - 1) + i], 128);
					}
				}

				upsizer->Resize(VXFullYF, VPitchY, VXSmallYF, nBlkXP);
				upsizer->Resize(VYFullYF, VPitchY, VYSmallYF, nBlkXP);

				if (d->vi.format->colorFamily != cmGray) {
					VectorSmallMaskYToHalfUV(VXSmallYF, nBlkXP, nBlkYP, VXSmallUVF, xRatioUV);
					VectorSmallMaskYToHalfUV(VYSmallYF, nBlkXP, nBlkYP, VYSmallUVF, yRatioUV);

					upsizerUV->Resize(VXFullUVF, VPitchUV, VXSmallUVF, nBlkXP);
					upsizerUV->Resize(VYFullUVF, VPitchUV, VYSmallUVF, nBlkXP);
				}
			}
			// analyze vectors field to detect occlusion
			//        double occNormF = time256/(256*ml);
			//        MakeVectorOcclusionMask(mvClipF, nBlkX, nBlkY, occNormF, 1.0, nPel, MaskSmallF, nBlkXP);
			MakeVectorOcclusionMaskTime(&ballsF, nBlkX, nBlkY, ml, 1.0, nPel, MaskSmallF, nBlkXP, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
			if (nBlkXP > nBlkX) // fill right
				for (int32_t j = 0; j<nBlkY; j++)
					MaskSmallF[j*nBlkXP + nBlkX] = MaskSmallF[j*nBlkXP + nBlkX - 1];
			if (nBlkYP > nBlkY) // fill bottom
				for (int32_t i = 0; i<nBlkXP; i++)
					MaskSmallF[nBlkXP*nBlkY + i] = MaskSmallF[nBlkXP*(nBlkY - 1) + i];

			upsizer->Resize(MaskFullYF, VPitchY, MaskSmallF, nBlkXP);
			if (d->vi.format->colorFamily != cmGray)
				upsizerUV->Resize(MaskFullUVF, VPitchUV, MaskSmallF, nBlkXP);

			d->nleftLast = nleft;

			if (maskmode == 2) { // These motion vectors should only be needed with maskmode 2. Why was the Avisynth plugin requesting them for all mask modes?
								 // Get motion info from more frames for occlusion areas
				const VSFrameRef *mvFF = vsapi->getFrameFilter(nleft, d->mvfw, frameCtx);
				ballsF.Update(mvFF);// forward from prev to cur
				isUsableF = ballsF.IsUsable();
				vsapi->freeFrame(mvFF);

				const VSFrameRef *mvBB = vsapi->getFrameFilter(nright, d->mvbw, frameCtx);
				ballsB.Update(mvBB);// backward from next next to next
				isUsableB = ballsB.IsUsable();
				vsapi->freeFrame(mvBB);
			}

			if (maskmode == 2 && isUsableB && isUsableF) {// slow method with extra frames
														  // get vector mask from extra frames
				MakeVectorSmallMasks(&ballsB, nBlkX, nBlkY, VXSmallYBB, nBlkXP, VYSmallYBB, nBlkXP);
				MakeVectorSmallMasks(&ballsF, nBlkX, nBlkY, VXSmallYFF, nBlkXP, VYSmallYFF, nBlkXP);
				if (nBlkXP > nBlkX) {// fill right
					for (int32_t j = 0; j<nBlkY; j++) {
						VXSmallYBB[j*nBlkXP + nBlkX] = VSMIN(VXSmallYBB[j*nBlkXP + nBlkX - 1], 128);
						VYSmallYBB[j*nBlkXP + nBlkX] = VYSmallYBB[j*nBlkXP + nBlkX - 1];
						VXSmallYFF[j*nBlkXP + nBlkX] = VSMIN(VXSmallYFF[j*nBlkXP + nBlkX - 1], 128);
						VYSmallYFF[j*nBlkXP + nBlkX] = VYSmallYFF[j*nBlkXP + nBlkX - 1];
					}
				}
				if (nBlkYP > nBlkY) {// fill bottom
					for (int32_t i = 0; i<nBlkXP; i++) {
						VXSmallYBB[nBlkXP*nBlkY + i] = VXSmallYBB[nBlkXP*(nBlkY - 1) + i];
						VYSmallYBB[nBlkXP*nBlkY + i] = VSMIN(VYSmallYBB[nBlkXP*(nBlkY - 1) + i], 128);
						VXSmallYFF[nBlkXP*nBlkY + i] = VXSmallYFF[nBlkXP*(nBlkY - 1) + i];
						VYSmallYFF[nBlkXP*nBlkY + i] = VSMIN(VYSmallYFF[nBlkXP*(nBlkY - 1) + i], 128);
					}
				}

				upsizer->Resize(VXFullYBB, VPitchY, VXSmallYBB, nBlkXP);
				upsizer->Resize(VYFullYBB, VPitchY, VYSmallYBB, nBlkXP);

				upsizer->Resize(VXFullYFF, VPitchY, VXSmallYFF, nBlkXP);
				upsizer->Resize(VYFullYFF, VPitchY, VYSmallYFF, nBlkXP);

				FlowInterExtra(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, pSrc[0] + nOffsetY, nRefPitches[0],
					VXFullYB, VXFullYF, VYFullYB, VYFullYF, MaskFullYB, MaskFullYF, VPitchY,
					nWidth, nHeight, time256, nPel, LUTVB, LUTVF, VXFullYBB, VXFullYFF, VYFullYBB, VYFullYFF);
				if (d->vi.format->colorFamily != cmGray) {
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
				}
			}
			else if (maskmode == 1) {// old method without extra frames
				FlowInter(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, pSrc[0] + nOffsetY, nRefPitches[0],
					VXFullYB, VXFullYF, VYFullYB, VYFullYF, MaskFullYB, MaskFullYF, VPitchY,
					nWidth, nHeight, time256, nPel, LUTVB, LUTVF);
				if (d->vi.format->colorFamily != cmGray) {
					FlowInter(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, pSrc[1] + nOffsetUV, nRefPitches[1],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF);
					FlowInter(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, pSrc[2] + nOffsetUV, nRefPitches[2],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF);
				}
			}
			else {// mode=0, faster simple method
				FlowInterSimple(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, pSrc[0] + nOffsetY, nRefPitches[0],
					VXFullYB, VXFullYF, VYFullYB, VYFullYF, MaskFullYB, MaskFullYF, VPitchY,
					nWidth, nHeight, time256, nPel, LUTVB, LUTVF);
				if (d->vi.format->colorFamily != cmGray) {
					FlowInterSimple(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, pSrc[1] + nOffsetUV, nRefPitches[1],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF);
					FlowInterSimple(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, pSrc[2] + nOffsetUV, nRefPitches[2],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, LUTVB, LUTVF);
				}
			}

			vsapi->freeFrame(src);
			vsapi->freeFrame(ref);

			return dst;
		}
		else { // poor estimation
			const VSFrameRef *src = vsapi->getFrameFilter(d->vi.numFrames ? VSMIN(nleft, d->vi.numFrames - 1) : nleft, d->node, frameCtx);

			if (blend) {//let's blend src with ref frames like ConvertFPS
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

				// blend with time weight
				Blend(pDst[0], pSrc[0], pRef[0], nHeight, nWidth, nDstPitches[0], nSrcPitches[0], nRefPitches[0], time256);
				if (d->vi.format->colorFamily != cmGray) {
					Blend(pDst[1], pSrc[1], pRef[1], nHeightUV, nWidthUV, nDstPitches[1], nSrcPitches[1], nRefPitches[1], time256);
					Blend(pDst[2], pSrc[2], pRef[2], nHeightUV, nWidthUV, nDstPitches[2], nSrcPitches[2], nRefPitches[2], time256);
				}

				vsapi->freeFrame(src);
				vsapi->freeFrame(ref);

				return dst;
			}
			else {
				return src; // like ChangeFPS
			}
		}
	}

	return nullptr;
}


static void VS_CC mvflowfpsFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVFlowFPSData *d = reinterpret_cast<MVFlowFPSData *>(instanceData);

	delete[] d->VXFullYB;
	delete[] d->VYFullYB;
	delete[] d->VXSmallYB;
	delete[] d->VYSmallYB;
	delete[] d->VXFullYF;
	delete[] d->VYFullYF;
	delete[] d->VXSmallYF;
	delete[] d->VYSmallYF;

	if (d->maskmode == 2) {
		delete[] d->VXFullYBB;
		delete[] d->VYFullYBB;
		delete[] d->VXSmallYBB;
		delete[] d->VYSmallYBB;
		delete[] d->VXFullYFF;
		delete[] d->VYFullYFF;
		delete[] d->VXSmallYFF;
		delete[] d->VYSmallYFF;
	}

	delete[] d->MaskSmallB;
	delete[] d->MaskFullYB;
	delete[] d->MaskSmallF;
	delete[] d->MaskFullYF;

	if (d->vi.format->colorFamily != cmGray) {
		delete[] d->VXFullUVB;
		delete[] d->VYFullUVB;
		delete[] d->VXSmallUVB;
		delete[] d->VYSmallUVB;
		delete[] d->VXFullUVF;
		delete[] d->VYFullUVF;
		delete[] d->VXSmallUVF;
		delete[] d->VYSmallUVF;

		if (d->maskmode == 2) {
			delete[] d->VXFullUVBB;
			delete[] d->VYFullUVBB;
			delete[] d->VXSmallUVBB;
			delete[] d->VYSmallUVBB;
			delete[] d->VXFullUVFF;
			delete[] d->VYFullUVFF;
			delete[] d->VXSmallUVFF;
			delete[] d->VYSmallUVFF;
		}

		delete[] d->MaskFullUVB;
		delete[] d->MaskFullUVF;

		delete d->upsizerUV;
	}

	delete[] d->LUTVB;
	delete[] d->LUTVF;

	delete d->mvClipB;
	delete d->mvClipF;

	delete d->bleh;

	delete d->upsizer;

	vsapi->freeNode(d->finest);
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


static void VS_CC mvflowfpsCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVFlowFPSData d;
	MVFlowFPSData *data;

	int err;

	d.num = vsapi->propGetInt(in, "num", 0, &err);
	if (err)
		d.num = 25;

	d.den = vsapi->propGetInt(in, "den", 0, &err);
	if (err)
		d.den = 1;

	d.maskmode = int64ToIntS(vsapi->propGetInt(in, "mask", 0, &err));
	if (err)
		d.maskmode = 2;

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


	if (d.maskmode < 0 || d.maskmode > 2) {
		vsapi->setError(out, "FlowFPS: mask must be 0, 1, or 2.");
		return;
	}

	if (d.ml <= 0.0) {
		vsapi->setError(out, "FlowFPS: ml must be greater than 0.");
		return;
	}


	d.super = vsapi->propGetNode(in, "super", 0, nullptr);

	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("FlowFPS: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
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
			vsapi->setError(out, "FlowFPS: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
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
		vsapi->setError(out, std::string("FlowFPS: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvbw);
		vsapi->freeNode(d.mvfw);
		return;
	}

	try {
		d.mvClipF = new MVClipDicks(d.mvfw, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("FlowFPS: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		return;
	}

	// XXX Alternatively, use both clips' delta as offsets in GetFrame.
	if (d.mvClipF->GetDeltaFrame() != d.mvClipB->GetDeltaFrame()) {
		vsapi->setError(out, "FlowFPS: mvbw and mvfw must be generated with the same delta.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}

	// Make sure the motion vector clips are correct.
	if (!d.mvClipB->IsBackward() || d.mvClipF->IsBackward()) {
		if (!d.mvClipB->IsBackward())
			vsapi->setError(out, "FlowFPS: mvbw must be generated with isb=True.");
		else
			vsapi->setError(out, "FlowFPS: mvfw must be generated with isb=False.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}

	try {
		d.bleh = new MVFilter(d.mvfw, "FlowFPS", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("FlowFPS: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}

	try {
		// So it checks the similarity of mvfw and mvfw? ?????
		// Copied straight from 2.5.11.3...
		d.bleh->CheckSimilarity(d.mvClipF, "mvfw");
		d.bleh->CheckSimilarity(d.mvClipB, "mvbw");
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("FlowFPS: ").append(e.what()).c_str());
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
	else
	{
		VSPlugin *mvtoolsPlugin = vsapi->getPluginById("com.nodame.mvsf", core);
		VSPlugin *stdPlugin = vsapi->getPluginById("com.vapoursynth.std", core);

		VSMap *args = vsapi->createMap();
		vsapi->propSetNode(args, "super", d.super, paReplace);
		VSMap *ret = vsapi->invoke(mvtoolsPlugin, "Finest", args);
		if (vsapi->getError(ret)) {
			vsapi->setError(out, std::string("FlowFPS: ").append(vsapi->getError(ret)).c_str());

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
			// prefix the error messages
			vsapi->setError(out, std::string("FlowFPS: ").append(vsapi->getError(ret)).c_str());

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
	d.vi = *vsapi->getVideoInfo(d.node);


	if (d.vi.fpsNum == 0 || d.vi.fpsDen == 0) {
		vsapi->setError(out, "FlowFPS: The input clip must have a frame rate. Invoke AssumeFPS if necessary.");
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

	int64_t numeratorOld = d.vi.fpsNum;
	int64_t denominatorOld = d.vi.fpsDen;
	int64_t numerator, denominator;

	if (d.num != 0 && d.den != 0) {
		numerator = d.num;
		denominator = d.den;
	}
	else {
		numerator = numeratorOld * 2; // double fps by default
		denominator = denominatorOld;
	}

	//  safe for big numbers since v2.1
	d.fa = denominator * numeratorOld;
	d.fb = numerator * denominatorOld;
	int64_t fgcd = gcd(d.fa, d.fb); // general common divisor
	d.fa /= fgcd;
	d.fb /= fgcd;

	setFPS(&d.vi, numerator, denominator);

	if (d.vi.numFrames)
		d.vi.numFrames = (int32_t)(1 + (d.vi.numFrames - 1) * d.fb / d.fa);


	if (d.bleh->nWidth != d.vi.width || d.bleh->nHeight != d.vi.height) {
		vsapi->setError(out, "FlowFPS: inconsistent source and vector frame size.");
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



	const VSVideoInfo *supervi = vsapi->getVideoInfo(d.super);
	int32_t nSuperWidth = supervi->width;

	if (d.bleh->nHeight != nHeightS || d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2) {
		vsapi->setError(out, "FlowFPS: wrong source or super clip frame size.");
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

	if (!((d.bleh->nWidth + d.bleh->nHPadding * 2) == supervi->width && (d.bleh->nHeight + d.bleh->nVPadding * 2) <= supervi->height)) {
		vsapi->setError(out, "FlowFPS: inconsistent clips frame size! Incomprehensible error messages are the best, right?");
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


	d.VXFullYB = new uint8_t[d.nHeightP * d.VPitchY];
	d.VYFullYB = new uint8_t[d.nHeightP * d.VPitchY];

	d.VXFullYF = new uint8_t[d.nHeightP * d.VPitchY];
	d.VYFullYF = new uint8_t[d.nHeightP * d.VPitchY];

	d.VXSmallYB = new uint8_t[d.nBlkXP * d.nBlkYP];
	d.VYSmallYB = new uint8_t[d.nBlkXP * d.nBlkYP];

	d.VXSmallYF = new uint8_t[d.nBlkXP * d.nBlkYP];
	d.VYSmallYF = new uint8_t[d.nBlkXP * d.nBlkYP];

	if (d.maskmode == 2) {
		d.VXFullYBB = new uint8_t[d.nHeightP * d.VPitchY];
		d.VYFullYBB = new uint8_t[d.nHeightP * d.VPitchY];

		d.VXFullYFF = new uint8_t[d.nHeightP * d.VPitchY];
		d.VYFullYFF = new uint8_t[d.nHeightP * d.VPitchY];

		d.VXSmallYBB = new uint8_t[d.nBlkXP * d.nBlkYP];
		d.VYSmallYBB = new uint8_t[d.nBlkXP * d.nBlkYP];

		d.VXSmallYFF = new uint8_t[d.nBlkXP * d.nBlkYP];
		d.VYSmallYFF = new uint8_t[d.nBlkXP * d.nBlkYP];
	}

	d.MaskSmallB = new uint8_t[d.nBlkXP * d.nBlkYP];
	d.MaskFullYB = new uint8_t[d.nHeightP * d.VPitchY];

	d.MaskSmallF = new uint8_t[d.nBlkXP * d.nBlkYP];
	d.MaskFullYF = new uint8_t[d.nHeightP * d.VPitchY];

	d.upsizer = new SimpleResize(d.nWidthP, d.nHeightP, d.nBlkXP, d.nBlkYP);

	if (d.vi.format->colorFamily != cmGray) {
		d.VXFullUVB = new uint8_t[d.nHeightPUV * d.VPitchUV];
		d.VYFullUVB = new uint8_t[d.nHeightPUV * d.VPitchUV];
		d.VXFullUVF = new uint8_t[d.nHeightPUV * d.VPitchUV];
		d.VYFullUVF = new uint8_t[d.nHeightPUV * d.VPitchUV];
		d.VXSmallUVB = new uint8_t[d.nBlkXP * d.nBlkYP];
		d.VYSmallUVB = new uint8_t[d.nBlkXP * d.nBlkYP];
		d.VXSmallUVF = new uint8_t[d.nBlkXP * d.nBlkYP];
		d.VYSmallUVF = new uint8_t[d.nBlkXP * d.nBlkYP];

		if (d.maskmode == 2) {
			d.VXFullUVBB = new uint8_t[d.nHeightPUV * d.VPitchUV];
			d.VYFullUVBB = new uint8_t[d.nHeightPUV * d.VPitchUV];
			d.VXFullUVFF = new uint8_t[d.nHeightPUV * d.VPitchUV];
			d.VYFullUVFF = new uint8_t[d.nHeightPUV * d.VPitchUV];
			d.VXSmallUVBB = new uint8_t[d.nBlkXP * d.nBlkYP];
			d.VYSmallUVBB = new uint8_t[d.nBlkXP * d.nBlkYP];
			d.VXSmallUVFF = new uint8_t[d.nBlkXP * d.nBlkYP];
			d.VYSmallUVFF = new uint8_t[d.nBlkXP * d.nBlkYP];
		}

		d.MaskFullUVB = new uint8_t[d.nHeightPUV * d.VPitchUV];
		d.MaskFullUVF = new uint8_t[d.nHeightPUV * d.VPitchUV];

		d.upsizerUV = new SimpleResize(d.nWidthPUV, d.nHeightPUV, d.nBlkXP, d.nBlkYP);
	}



	d.LUTVB = new int32_t[256];
	d.LUTVF = new int32_t[256];

	d.nleftLast = -1000;
	d.nrightLast = -1000;


	data = new MVFlowFPSData;
	*data = d;

	// Can't use fmParallel because of nleftLast/nrightLast.
	vsapi->createFilter(in, out, "FlowFPS", mvflowfpsInit, mvflowfpsGetFrame, mvflowfpsFree, fmParallelRequests, 0, data, core);

	// AssumeFPS sets the _DurationNum and _DurationDen properties.
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
		vsapi->setError(out, std::string("FlowFPS: Failed to invoke AssumeFPS. Error message: ").append(error).c_str());
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
		vsapi->setError(out, std::string("FlowFPS: Failed to invoke Cache. Error message: ").append(error).c_str());
		vsapi->freeMap(ret);
		return;
	}
	node = vsapi->propGetNode(ret, "clip", 0, nullptr);
	vsapi->freeMap(ret);
	vsapi->propSetNode(out, "clip", node, paReplace);
	vsapi->freeNode(node);
}

void mvflowfpsRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("FlowFPS",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"num:int:opt;"
		"den:int:opt;"
		"mask:int:opt;"
		"ml:float:opt;"
		"blend:int:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvflowfpsCreate, 0, plugin);
}
