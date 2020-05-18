#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "VapourSynth.h"
#include "VSHelper.h"
#include "MaskFun.hpp"
#include "MVFilter.hpp"
#include "MVInterface.h"
#include "SimpleResize.hpp"

struct MVFlowInterData {
	VSNodeRef *node;
	const VSVideoInfo *vi;
	VSNodeRef *finest;
	VSNodeRef *super;
	VSNodeRef *mvbw;
	VSNodeRef *mvfw;
	double time;
	double ml;
	bool blend;
	double thscd1;
	double thscd2;
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
	SimpleResize<int32_t> *upsizer;
	SimpleResize<double> *upsizer2;
	SimpleResize<int32_t> *upsizerUV;
	SimpleResize<double> *upsizerUV2;
};

static void VS_CC mvflowinterInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData *d = reinterpret_cast<MVFlowInterData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvflowinterGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData *d = reinterpret_cast<MVFlowInterData *>(*instanceData);

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
			const int32_t nHeightP = d->nHeightP;
			const int32_t nHeightPUV = d->nHeightPUV;
			const int32_t nBlkXP = d->nBlkXP;
			const int32_t nBlkYP = d->nBlkYP;
			SimpleResize<int32_t> *upsizer = d->upsizer;
			SimpleResize<double> *upsizer2 = d->upsizer2;
			SimpleResize<int32_t> *upsizerUV = d->upsizerUV;
			SimpleResize<double> *upsizerUV2 = d->upsizerUV2;

			int32_t nOffsetY = nRefPitches[0] * nVPadding * nPel + nHPadding * bytesPerSample * nPel;
			int32_t nOffsetUV = nRefPitches[1] * nVPaddingUV * nPel + nHPaddingUV * bytesPerSample * nPel;


			auto VXFullYB = new int32_t[nHeightP * VPitchY];
			auto VYFullYB = new int32_t[nHeightP * VPitchY];
			auto VXFullYF = new int32_t[nHeightP * VPitchY];
			auto VYFullYF = new int32_t[nHeightP * VPitchY];
			auto VXSmallYB = new int32_t[nBlkXP * nBlkYP];
			auto VYSmallYB = new int32_t[nBlkXP * nBlkYP];
			auto VXSmallYF = new int32_t[nBlkXP * nBlkYP];
			auto VYSmallYF = new int32_t[nBlkXP * nBlkYP];
			auto MaskSmallB = new double[nBlkXP * nBlkYP];
			auto MaskFullYB = new double[nHeightP * VPitchY];
			auto MaskSmallF = new double[nBlkXP * nBlkYP];
			auto MaskFullYF = new double[nHeightP * VPitchY];
			int32_t *VXFullUVB = nullptr;
			int32_t *VYFullUVB = nullptr;
			int32_t *VXFullUVF = nullptr;
			int32_t *VYFullUVF = nullptr;
			int32_t *VXSmallUVB = nullptr;
			int32_t *VYSmallUVB = nullptr;
			int32_t *VXSmallUVF = nullptr;
			int32_t *VYSmallUVF = nullptr;
			double *MaskFullUVB = nullptr;
			double *MaskFullUVF = nullptr;

			MakeVectorSmallMasks(ballsB, nBlkX, nBlkY, VXSmallYB, nBlkXP, VYSmallYB, nBlkXP);
			MakeVectorSmallMasks(ballsF, nBlkX, nBlkY, VXSmallYF, nBlkXP, VYSmallYF, nBlkXP);
			CheckAndPadSmallY(VXSmallYB, VYSmallYB, nBlkXP, nBlkYP, nBlkX, nBlkY);
			CheckAndPadSmallY(VXSmallYF, VYSmallYF, nBlkXP, nBlkYP, nBlkX, nBlkY);
			// analyze vectors field to detect occlusion
			//      double occNormB = (256-time256)/(256*ml);
			MakeVectorOcclusionMaskTime(ballsB, true, nBlkX, nBlkY, ml, 1.0, nPel, MaskSmallB, nBlkXP, (256 - time256), nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
			//      double occNormF = time256/(256*ml);
			MakeVectorOcclusionMaskTime(ballsF, false, nBlkX, nBlkY, ml, 1.0, nPel, MaskSmallF, nBlkXP, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
			CheckAndPadMaskSmall(MaskSmallB, nBlkXP, nBlkYP, nBlkX, nBlkY);
			CheckAndPadMaskSmall(MaskSmallF, nBlkXP, nBlkYP, nBlkX, nBlkY);

			// upsize (bilinear interpolate) vector masks to fullframe size
			upsizer->Resize(VXFullYB, VPitchY, VXSmallYB, nBlkXP, true);
			upsizer->Resize(VYFullYB, VPitchY, VYSmallYB, nBlkXP, false);
			upsizer->Resize(VXFullYF, VPitchY, VXSmallYF, nBlkXP, true);
			upsizer->Resize(VYFullYF, VPitchY, VYSmallYF, nBlkXP, false);
			upsizer2->Resize(MaskFullYB, VPitchY, MaskSmallB, nBlkXP, false);
			upsizer2->Resize(MaskFullYF, VPitchY, MaskSmallF, nBlkXP, false);

			if (d->vi->format->colorFamily != cmGray) {
				VXFullUVB = new int32_t[nHeightPUV * VPitchUV];
				VYFullUVB = new int32_t[nHeightPUV * VPitchUV];
				VXFullUVF = new int32_t[nHeightPUV * VPitchUV];
				VYFullUVF = new int32_t[nHeightPUV * VPitchUV];
				VXSmallUVB = new int32_t[nBlkXP * nBlkYP];
				VYSmallUVB = new int32_t[nBlkXP * nBlkYP];
				VXSmallUVF = new int32_t[nBlkXP * nBlkYP];
				VYSmallUVF = new int32_t[nBlkXP * nBlkYP];
				MaskFullUVB = new double[nHeightPUV * VPitchUV];
				MaskFullUVF = new double[nHeightPUV * VPitchUV];

				VectorSmallMaskYToHalfUV(VXSmallYB, nBlkXP, nBlkYP, VXSmallUVB, xRatioUV);
				VectorSmallMaskYToHalfUV(VYSmallYB, nBlkXP, nBlkYP, VYSmallUVB, yRatioUV);
				VectorSmallMaskYToHalfUV(VXSmallYF, nBlkXP, nBlkYP, VXSmallUVF, xRatioUV);
				VectorSmallMaskYToHalfUV(VYSmallYF, nBlkXP, nBlkYP, VYSmallUVF, yRatioUV);

				upsizerUV->Resize(VXFullUVB, VPitchUV, VXSmallUVB, nBlkXP, true);
				upsizerUV->Resize(VYFullUVB, VPitchUV, VYSmallUVB, nBlkXP, false);
				upsizerUV->Resize(VXFullUVF, VPitchUV, VXSmallUVF, nBlkXP, true);
				upsizerUV->Resize(VYFullUVF, VPitchUV, VYSmallUVF, nBlkXP, false);
				upsizerUV2->Resize(MaskFullUVB, VPitchUV, MaskSmallB, nBlkXP, false);
				upsizerUV2->Resize(MaskFullUVF, VPitchUV, MaskSmallF, nBlkXP, false);
			}


			const VSFrameRef *mvFF = vsapi->getFrameFilter(n, d->mvfw, frameCtx);
			ballsF.Update(mvFF);
			vsapi->freeFrame(mvFF);

			const VSFrameRef *mvBB = vsapi->getFrameFilter(n + off, d->mvbw, frameCtx);
			ballsB.Update(mvBB);
			vsapi->freeFrame(mvBB);


			if (ballsB.IsUsable() && ballsF.IsUsable())
			{
				auto VXFullYBB = new int32_t[nHeightP * VPitchY];
				auto VYFullYBB = new int32_t[nHeightP * VPitchY];
				auto VXFullYFF = new int32_t[nHeightP * VPitchY];
				auto VYFullYFF = new int32_t[nHeightP * VPitchY];
				auto VXSmallYBB = new int32_t[nBlkXP * nBlkYP];
				auto VYSmallYBB = new int32_t[nBlkXP * nBlkYP];
				auto VXSmallYFF = new int32_t[nBlkXP * nBlkYP];
				auto VYSmallYFF = new int32_t[nBlkXP * nBlkYP];

				// get vector mask from extra frames
				MakeVectorSmallMasks(ballsB, nBlkX, nBlkY, VXSmallYBB, nBlkXP, VYSmallYBB, nBlkXP);
				MakeVectorSmallMasks(ballsF, nBlkX, nBlkY, VXSmallYFF, nBlkXP, VYSmallYFF, nBlkXP);
				CheckAndPadSmallY(VXSmallYBB, VYSmallYBB, nBlkXP, nBlkYP, nBlkX, nBlkY);
				CheckAndPadSmallY(VXSmallYFF, VYSmallYFF, nBlkXP, nBlkYP, nBlkX, nBlkY);

				// upsize vectors to full frame
				upsizer->Resize(VXFullYBB, VPitchY, VXSmallYBB, nBlkXP, true);
				upsizer->Resize(VYFullYBB, VPitchY, VYSmallYBB, nBlkXP, false);
				upsizer->Resize(VXFullYFF, VPitchY, VXSmallYFF, nBlkXP, true);
				upsizer->Resize(VYFullYFF, VPitchY, VYSmallYFF, nBlkXP, false);

				FlowInterExtra(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, pSrc[0] + nOffsetY, nRefPitches[0],
					VXFullYB, VXFullYF, VYFullYB, VYFullYF, MaskFullYB, MaskFullYF, VPitchY,
					nWidth, nHeight, time256, nPel, VXFullYBB, VXFullYFF, VYFullYBB, VYFullYFF);

				if (d->vi->format->colorFamily != cmGray) {
					auto VXFullUVFF = new int32_t[nHeightPUV * VPitchUV];
					auto VXFullUVBB = new int32_t[nHeightPUV * VPitchUV];
					auto VYFullUVBB = new int32_t[nHeightPUV * VPitchUV];
					auto VYFullUVFF = new int32_t[nHeightPUV * VPitchUV];
					auto VXSmallUVBB = new int32_t[nBlkXP * nBlkYP];
					auto VYSmallUVBB = new int32_t[nBlkXP * nBlkYP];
					auto VXSmallUVFF = new int32_t[nBlkXP * nBlkYP];
					auto VYSmallUVFF = new int32_t[nBlkXP * nBlkYP];

					VectorSmallMaskYToHalfUV(VXSmallYBB, nBlkXP, nBlkYP, VXSmallUVBB, xRatioUV);
					VectorSmallMaskYToHalfUV(VYSmallYBB, nBlkXP, nBlkYP, VYSmallUVBB, yRatioUV);
					VectorSmallMaskYToHalfUV(VXSmallYFF, nBlkXP, nBlkYP, VXSmallUVFF, xRatioUV);
					VectorSmallMaskYToHalfUV(VYSmallYFF, nBlkXP, nBlkYP, VYSmallUVFF, yRatioUV);

					upsizerUV->Resize(VXFullUVBB, VPitchUV, VXSmallUVBB, nBlkXP, true);
					upsizerUV->Resize(VYFullUVBB, VPitchUV, VYSmallUVBB, nBlkXP, false);
					upsizerUV->Resize(VXFullUVFF, VPitchUV, VXSmallUVFF, nBlkXP, true);
					upsizerUV->Resize(VYFullUVFF, VPitchUV, VYSmallUVFF, nBlkXP, false);

					FlowInterExtra(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, pSrc[1] + nOffsetUV, nRefPitches[1],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, VXFullUVBB, VXFullUVFF, VYFullUVBB, VYFullUVFF);
					FlowInterExtra(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, pSrc[2] + nOffsetUV, nRefPitches[2],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel, VXFullUVBB, VXFullUVFF, VYFullUVBB, VYFullUVFF);

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
					nWidth, nHeight, time256, nPel);
				if (d->vi->format->colorFamily != cmGray) {
					FlowInter(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, pSrc[1] + nOffsetUV, nRefPitches[1],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel);
					FlowInter(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, pSrc[2] + nOffsetUV, nRefPitches[2],
						VXFullUVB, VXFullUVF, VYFullUVB, VYFullUVF, MaskFullUVB, MaskFullUVF, VPitchUV,
						nWidthUV, nHeightUV, time256, nPel);
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

	return nullptr;
}


static void VS_CC mvflowinterFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData *d = reinterpret_cast<MVFlowInterData *>(instanceData);

	delete d->mvClipB;
	delete d->mvClipF;

	delete d->bleh;

	delete d->upsizer;
	delete d->upsizer2;
	if (d->vi->format->colorFamily != cmGray) {
		delete d->upsizerUV;
		delete d->upsizerUV2;
	}
	vsapi->freeNode(d->finest);
	vsapi->freeNode(d->super);
	vsapi->freeNode(d->mvfw);
	vsapi->freeNode(d->mvbw);
	vsapi->freeNode(d->node);
	delete d;
}


static void VS_CC mvflowinterCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVFlowInterData d;
	MVFlowInterData *data;

	int err;

	d.time = vsapi->propGetFloat(in, "time", 0, &err);
	if (err)
		d.time = 50.;

	d.ml = vsapi->propGetFloat(in, "ml", 0, &err);
	if (err)
		d.ml = 100.;

	d.blend = !!vsapi->propGetInt(in, "blend", 0, &err);
	if (err)
		d.blend = 1;

	d.thscd1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.thscd1 = MV_DEFAULT_SCD1;

	d.thscd2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.thscd2 = MV_DEFAULT_SCD2;


	if (d.time < 0. || d.time > 100.) {
		vsapi->setError(out, "FlowInter: time must be between 0 and 100 % (inclusive).");
		return;
	}

	if (d.ml <= 0.) {
		vsapi->setError(out, "FlowInter: ml must be greater than 0.");
		return;
	}

	d.time256 = (int32_t)(d.time * 256. / 100.);


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
		VSPlugin *mvtoolsPlugin = vsapi->getPluginById("com.zonked.mvsf", core);
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
	if (!isConstantFormat(d.vi) || d.vi->format->bitsPerSample < 32 || d.vi->format->sampleType != stFloat) {
		vsapi->setError(out, "FlowInter: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.finest);
		vsapi->freeNode(d.mvfw);
		vsapi->freeNode(d.mvbw);
		vsapi->freeNode(d.node);
		delete d.bleh;
		delete d.mvClipB;
		delete d.mvClipF;
		return;
	}
	d.nBlkXP = d.bleh->nBlkX;
	while (d.nBlkXP * (d.bleh->nBlkSizeX - d.bleh->nOverlapX) + d.bleh->nOverlapX < d.bleh->nWidth)
		d.nBlkXP++;
	d.nBlkYP = d.bleh->nBlkY;
	while (d.nBlkYP * (d.bleh->nBlkSizeY - d.bleh->nOverlapY) + d.bleh->nOverlapY < d.bleh->nHeight)
		d.nBlkYP++;
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
	d.upsizer = new SimpleResize<int32_t>(d.nWidthP, d.nHeightP, d.nBlkXP, d.nBlkYP, d.mvClipB->nWidth, d.mvClipB->nHeight, d.mvClipB->nPel);
	d.upsizer2 = new SimpleResize<double>(d.nWidthP, d.nHeightP, d.nBlkXP, d.nBlkYP, 0, 0, 0);
	if (d.vi->format->colorFamily != cmGray) {
		d.upsizerUV = new SimpleResize<int32_t>(d.nWidthPUV, d.nHeightPUV, d.nBlkXP, d.nBlkYP, d.nWidthUV, d.nHeightUV, d.mvClipB->nPel);
		d.upsizerUV2 = new SimpleResize<double>(d.nWidthPUV, d.nHeightPUV, d.nBlkXP, d.nBlkYP, 0, 0, 0);
	}
	data = new MVFlowInterData;
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
