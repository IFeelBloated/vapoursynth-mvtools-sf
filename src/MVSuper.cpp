#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "VapourSynth.h"
#include "VSHelper.h"
#include "MVFrame.h"
#include "MVSuper.h"

struct MVSuperData {
	VSNodeRef *node;
	VSVideoInfo vi;
	VSNodeRef *pelclip;
	int32_t nHPad;
	int32_t nVPad;
	int32_t nPel;
	int32_t nLevels;
	int32_t sharp;
	int32_t rfilter;
	int32_t nWidth;
	int32_t nHeight;
	int32_t yRatioUV;
	int32_t xRatioUV;
	bool chroma;
	bool usePelClip;
	int32_t nSuperWidth;
	int32_t nSuperHeight;
	MVPlaneSet nModeYUV;
	bool isPelClipPadded;
};

static void VS_CC mvsuperInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVSuperData *d = reinterpret_cast<MVSuperData *>(*instanceData);
	vsapi->setVideoInfo(&d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvsuperGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVSuperData *d = reinterpret_cast<MVSuperData *>(*instanceData);
	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->node, frameCtx);
		if (d->usePelClip)
			vsapi->requestFrameFilter(n, d->pelclip, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		const uint8_t *pSrc[3] = { nullptr };
		uint8_t *pDst[3] = { nullptr };
		const uint8_t *pSrcPel[3] = { nullptr };
		int32_t nSrcPitch[3] = { 0 };
		int32_t nDstPitch[3] = { 0 };
		int32_t nSrcPelPitch[3] = { 0 };
		const VSFrameRef *srcPel = nullptr;
		if (d->usePelClip)
			srcPel = vsapi->getFrameFilter(n, d->pelclip, frameCtx);
		VSFrameRef *dst = vsapi->newVideoFrame(d->vi.format, d->vi.width, d->vi.height, src, core);
		for (int32_t plane = 0; plane < d->vi.format->numPlanes; ++plane) {
			pSrc[plane] = vsapi->getReadPtr(src, plane);
			nSrcPitch[plane] = vsapi->getStride(src, plane);
			pDst[plane] = vsapi->getWritePtr(dst, plane);
			nDstPitch[plane] = vsapi->getStride(dst, plane);
			memset(pDst[plane], 0, nDstPitch[plane] * vsapi->getFrameHeight(dst, plane));
		}
		MVGroupOfFrames *pSrcGOF = new MVGroupOfFrames(d->nLevels, d->nWidth, d->nHeight, d->nPel, d->nHPad, d->nVPad, d->nModeYUV, d->xRatioUV, d->yRatioUV);
		pSrcGOF->Update(d->nModeYUV, pDst[0], nDstPitch[0], pDst[1], nDstPitch[1], pDst[2], nDstPitch[2]);
		MVPlaneSet planes[3] = { YPLANE, UPLANE, VPLANE };
		for (int32_t plane = 0; plane < d->vi.format->numPlanes; ++plane)
			pSrcGOF->SetPlane(pSrc[plane], nSrcPitch[plane], planes[plane]);
		pSrcGOF->Reduce(d->nModeYUV, d->rfilter);
		pSrcGOF->Pad(d->nModeYUV);
		if (d->usePelClip) {
			MVFrame *srcFrames = pSrcGOF->GetFrame(0);
			for (int32_t plane = 0; plane < d->vi.format->numPlanes; ++plane) {
				pSrcPel[plane] = vsapi->getReadPtr(srcPel, plane);
				nSrcPelPitch[plane] = vsapi->getStride(srcPel, plane);
				MVPlane *srcPlane = srcFrames->GetPlane(planes[plane]);
				if (d->nModeYUV & planes[plane])
					srcPlane->RefineExt(pSrcPel[plane], nSrcPelPitch[plane], d->isPelClipPadded);
			}
		}
		else
			pSrcGOF->Refine(d->nModeYUV, d->sharp);
		vsapi->freeFrame(src);
		if (d->usePelClip)
			vsapi->freeFrame(srcPel);
		delete pSrcGOF;
		if (n == 0) {
			VSMap *props = vsapi->getFramePropsRW(dst);
			vsapi->propSetInt(props, "Super_height", d->nHeight, paReplace);
			vsapi->propSetInt(props, "Super_hpad", d->nHPad, paReplace);
			vsapi->propSetInt(props, "Super_vpad", d->nVPad, paReplace);
			vsapi->propSetInt(props, "Super_pel", d->nPel, paReplace);
			vsapi->propSetInt(props, "Super_modeyuv", d->nModeYUV, paReplace);
			vsapi->propSetInt(props, "Super_levels", d->nLevels, paReplace);
		}
		return dst;
	}
	return nullptr;
}

static void VS_CC mvsuperFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVSuperData *d = reinterpret_cast<MVSuperData *>(instanceData);
	vsapi->freeNode(d->node);
	delete d;
}

static void VS_CC mvsuperCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVSuperData d;
	MVSuperData *data;
	int err;
	d.nHPad = int64ToIntS(vsapi->propGetInt(in, "hpad", 0, &err));
	if (err)
		d.nHPad = 8;
	d.nVPad = int64ToIntS(vsapi->propGetInt(in, "vpad", 0, &err));
	if (err)
		d.nVPad = 8;
	d.nPel = int64ToIntS(vsapi->propGetInt(in, "pel", 0, &err));
	if (err)
		d.nPel = 2;
	d.nLevels = int64ToIntS(vsapi->propGetInt(in, "levels", 0, &err));
	d.chroma = !!vsapi->propGetInt(in, "chroma", 0, &err);
	if (err)
		d.chroma = 1;
	d.sharp = int64ToIntS(vsapi->propGetInt(in, "sharp", 0, &err));
	if (err)
		d.sharp = 2;
	d.rfilter = int64ToIntS(vsapi->propGetInt(in, "rfilter", 0, &err));
	if (err)
		d.rfilter = 2;
	if ((d.nPel != 1) && (d.nPel != 2) && (d.nPel != 4)) {
		vsapi->setError(out, "Super: pel must be 1, 2, or 4.");
		return;
	}
	if (d.sharp < 0 || d.sharp > 2) {
		vsapi->setError(out, "Super: sharp must be between 0 and 2 (inclusive).");
		return;
	}
	if (d.rfilter < 0 || d.rfilter > 4) {
		vsapi->setError(out, "Super: rfilter must be between 0 and 4 (inclusive).");
		return;
	}
	d.node = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = *vsapi->getVideoInfo(d.node);
	d.nWidth = d.vi.width;
	d.nHeight = d.vi.height;
	if (!isConstantFormat(&d.vi) || d.vi.format->bitsPerSample < 32 || d.vi.format->sampleType != stFloat) {
		vsapi->setError(out, "Super: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.node);
		return;
	}
	if (d.vi.format->colorFamily == cmGray)
		d.chroma = 0;
	if (d.vi.format->colorFamily == cmRGB)
		d.chroma = 1;
	d.nModeYUV = d.chroma ? YUVPLANES : YPLANE;
	d.xRatioUV = 1 << d.vi.format->subSamplingW;
	d.yRatioUV = 1 << d.vi.format->subSamplingH;
	int32_t nLevelsMax = 0;
	while (PlaneHeightLuma(d.vi.height, nLevelsMax, d.yRatioUV, d.nVPad) >= d.yRatioUV * 2 &&
		PlaneWidthLuma(d.vi.width, nLevelsMax, d.xRatioUV, d.nHPad) >= d.xRatioUV * 2) {
		++nLevelsMax;
	}
	if (d.nLevels <= 0 || d.nLevels > nLevelsMax)
		d.nLevels = nLevelsMax;
	d.pelclip = vsapi->propGetNode(in, "pelclip", 0, &err);
	const VSVideoInfo *pelvi = d.pelclip ? vsapi->getVideoInfo(d.pelclip) : nullptr;
	if (d.pelclip && (!isConstantFormat(pelvi) || pelvi->format != d.vi.format)) {
		vsapi->setError(out, "Super: pelclip must have the same format as the input clip, and it must have constant dimensions.");
		vsapi->freeNode(d.node);
		vsapi->freeNode(d.pelclip);
		return;
	}
	d.usePelClip = false;
	if (d.pelclip && (d.nPel >= 2)) {
		if ((pelvi->width == d.vi.width * d.nPel) &&
			(pelvi->height == d.vi.height * d.nPel)) {
			d.usePelClip = true;
			d.isPelClipPadded = false;
		}
		else if ((pelvi->width == (d.vi.width + d.nHPad * 2) * d.nPel) &&
			(pelvi->height == (d.vi.height + d.nVPad * 2) * d.nPel)) {
			d.usePelClip = true;
			d.isPelClipPadded = true;
		}
		else {
			vsapi->setError(out, "Super: pelclip's dimensions must be multiples of the input clip's dimensions.");
			vsapi->freeNode(d.pelclip);
			vsapi->freeNode(d.node);
			return;
		}
	}
	d.nSuperWidth = d.nWidth + 2 * d.nHPad;
	d.nSuperHeight = PlaneSuperOffset(false, d.nHeight, d.nLevels, d.nPel, d.nVPad, d.nSuperWidth, d.yRatioUV) / d.nSuperWidth;
	if (d.yRatioUV == 2 && d.nSuperHeight & 1)
		++d.nSuperHeight;
	if (d.xRatioUV == 2 && d.nSuperWidth & 1)
		++d.nSuperWidth;
	d.vi.width = d.nSuperWidth;
	d.vi.height = d.nSuperHeight;
	data = new MVSuperData;
	*data = d;
	vsapi->createFilter(in, out, "Super", mvsuperInit, mvsuperGetFrame, mvsuperFree, fmParallel, 0, data, core);
}

void mvsuperRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Super",
		"clip:clip;"
		"hpad:int:opt;"
		"vpad:int:opt;"
		"pel:int:opt;"
		"levels:int:opt;"
		"chroma:int:opt;"
		"sharp:int:opt;"
		"rfilter:int:opt;"
		"pelclip:clip:opt;"
		, mvsuperCreate, 0, plugin);
}
