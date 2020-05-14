#pragma once
#include "Include/VapourSynth.h"
#include "Include/VSHelper.h"
#include "MaskFun.hpp"

struct MVFinestData {
	VSNodeRef *super;
	VSVideoInfo vi;
	int32_t nWidth;
	int32_t nHeight;
	int32_t nSuperHPad;
	int32_t nSuperVPad;
	int32_t nSuperPel;
	int32_t nSuperModeYUV;
	int32_t nSuperLevels;
	int32_t nPel;
	int32_t xRatioUV;
	int32_t yRatioUV;
};

static void VS_CC mvfinestInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVFinestData *d = reinterpret_cast<MVFinestData *>(*instanceData);
	vsapi->setVideoInfo(&d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvfinestGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVFinestData *d = reinterpret_cast<MVFinestData *>(*instanceData);
	if (activationReason == arInitial)
		vsapi->requestFrameFilter(n, d->super, frameCtx);
	else if (activationReason == arAllFramesReady) {
		const VSFrameRef *ref = vsapi->getFrameFilter(n, d->super, frameCtx);
		VSFrameRef *dst = vsapi->newVideoFrame(d->vi.format, d->vi.width, d->vi.height, ref, core);
		uint8_t *pDst[3];
		const uint8_t *pRef[3];
		int32_t nDstPitches[3], nRefPitches[3];
		for (int32_t i = 0; i < d->vi.format->numPlanes; i++) {
			pDst[i] = vsapi->getWritePtr(dst, i);
			pRef[i] = vsapi->getReadPtr(ref, i);
			nDstPitches[i] = vsapi->getStride(dst, i);
			nRefPitches[i] = vsapi->getStride(ref, i);
		}
		int32_t bitsPerSample = d->vi.format->bitsPerSample;
		int32_t bytesPerSample = d->vi.format->bytesPerSample;
		if (d->nPel == 1) {
			for (int32_t i = 0; i < d->vi.format->numPlanes; i++)
				vs_bitblt(pDst[i], nDstPitches[i], pRef[i], nRefPitches[i], d->vi.width * bytesPerSample, d->vi.height);
		}
		else {
			MVGroupOfFrames *pRefGOF = new MVGroupOfFrames(d->nSuperLevels, d->nWidth, d->nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, d->xRatioUV, d->yRatioUV);
			pRefGOF->Update(d->nSuperModeYUV, (uint8_t*)pRef[0], nRefPitches[0], (uint8_t*)pRef[1], nRefPitches[1], (uint8_t*)pRef[2], nRefPitches[2]);
			MVPlane *pPlanes[3] = { 0 };
			pPlanes[0] = pRefGOF->GetFrame(0)->GetPlane(YPLANE);
			pPlanes[1] = pRefGOF->GetFrame(0)->GetPlane(UPLANE);
			pPlanes[2] = pRefGOF->GetFrame(0)->GetPlane(VPLANE);
			if (d->nPel == 2) {
				Merge4PlanesToBig(pDst[0], nDstPitches[0], pPlanes[0]->GetAbsolutePointer(0, 0),
					pPlanes[0]->GetAbsolutePointer(1, 0), pPlanes[0]->GetAbsolutePointer(0, 1),
					pPlanes[0]->GetAbsolutePointer(1, 1), pPlanes[0]->GetExtendedWidth(),
					pPlanes[0]->GetExtendedHeight(), pPlanes[0]->GetPitch());
				if (pPlanes[1])
					Merge4PlanesToBig(pDst[1], nDstPitches[1], pPlanes[1]->GetAbsolutePointer(0, 0),
						pPlanes[1]->GetAbsolutePointer(1, 0), pPlanes[1]->GetAbsolutePointer(0, 1),
						pPlanes[1]->GetAbsolutePointer(1, 1), pPlanes[1]->GetExtendedWidth(),
						pPlanes[1]->GetExtendedHeight(), pPlanes[1]->GetPitch());
				if (pPlanes[2])
					Merge4PlanesToBig(pDst[2], nDstPitches[2], pPlanes[2]->GetAbsolutePointer(0, 0),
						pPlanes[2]->GetAbsolutePointer(1, 0), pPlanes[2]->GetAbsolutePointer(0, 1),
						pPlanes[2]->GetAbsolutePointer(1, 1), pPlanes[2]->GetExtendedWidth(),
						pPlanes[2]->GetExtendedHeight(), pPlanes[2]->GetPitch());
			}
			else if (d->nPel == 4) {
				Merge16PlanesToBig(pDst[0], nDstPitches[0],
					pPlanes[0]->GetAbsolutePointer(0, 0), pPlanes[0]->GetAbsolutePointer(1, 0),
					pPlanes[0]->GetAbsolutePointer(2, 0), pPlanes[0]->GetAbsolutePointer(3, 0),
					pPlanes[0]->GetAbsolutePointer(0, 1), pPlanes[0]->GetAbsolutePointer(1, 1),
					pPlanes[0]->GetAbsolutePointer(2, 1), pPlanes[0]->GetAbsolutePointer(3, 1),
					pPlanes[0]->GetAbsolutePointer(0, 2), pPlanes[0]->GetAbsolutePointer(1, 2),
					pPlanes[0]->GetAbsolutePointer(2, 2), pPlanes[0]->GetAbsolutePointer(3, 2),
					pPlanes[0]->GetAbsolutePointer(0, 3), pPlanes[0]->GetAbsolutePointer(1, 3),
					pPlanes[0]->GetAbsolutePointer(2, 3), pPlanes[0]->GetAbsolutePointer(3, 3),
					pPlanes[0]->GetExtendedWidth(), pPlanes[0]->GetExtendedHeight(), pPlanes[0]->GetPitch());
				if (pPlanes[1])
					Merge16PlanesToBig(pDst[1], nDstPitches[1],
						pPlanes[1]->GetAbsolutePointer(0, 0), pPlanes[1]->GetAbsolutePointer(1, 0),
						pPlanes[1]->GetAbsolutePointer(2, 0), pPlanes[1]->GetAbsolutePointer(3, 0),
						pPlanes[1]->GetAbsolutePointer(0, 1), pPlanes[1]->GetAbsolutePointer(1, 1),
						pPlanes[1]->GetAbsolutePointer(2, 1), pPlanes[1]->GetAbsolutePointer(3, 1),
						pPlanes[1]->GetAbsolutePointer(0, 2), pPlanes[1]->GetAbsolutePointer(1, 2),
						pPlanes[1]->GetAbsolutePointer(2, 2), pPlanes[1]->GetAbsolutePointer(3, 2),
						pPlanes[1]->GetAbsolutePointer(0, 3), pPlanes[1]->GetAbsolutePointer(1, 3),
						pPlanes[1]->GetAbsolutePointer(2, 3), pPlanes[1]->GetAbsolutePointer(3, 3),
						pPlanes[1]->GetExtendedWidth(), pPlanes[1]->GetExtendedHeight(), pPlanes[1]->GetPitch());
				if (pPlanes[2])
					Merge16PlanesToBig(pDst[2], nDstPitches[2],
						pPlanes[2]->GetAbsolutePointer(0, 0), pPlanes[2]->GetAbsolutePointer(1, 0),
						pPlanes[2]->GetAbsolutePointer(2, 0), pPlanes[2]->GetAbsolutePointer(3, 0),
						pPlanes[2]->GetAbsolutePointer(0, 1), pPlanes[2]->GetAbsolutePointer(1, 1),
						pPlanes[2]->GetAbsolutePointer(2, 1), pPlanes[2]->GetAbsolutePointer(3, 1),
						pPlanes[2]->GetAbsolutePointer(0, 2), pPlanes[2]->GetAbsolutePointer(1, 2),
						pPlanes[2]->GetAbsolutePointer(2, 2), pPlanes[2]->GetAbsolutePointer(3, 2),
						pPlanes[2]->GetAbsolutePointer(0, 3), pPlanes[2]->GetAbsolutePointer(1, 3),
						pPlanes[2]->GetAbsolutePointer(2, 3), pPlanes[2]->GetAbsolutePointer(3, 3),
						pPlanes[2]->GetExtendedWidth(), pPlanes[2]->GetExtendedHeight(), pPlanes[2]->GetPitch());
			}
			delete pRefGOF;
		}
		vsapi->freeFrame(ref);
		return dst;
	}
	return nullptr;
}

static void VS_CC mvfinestFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVFinestData *d = reinterpret_cast<MVFinestData *>(instanceData);
	vsapi->freeNode(d->super);
	delete d;
}

static void VS_CC mvfinestCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVFinestData d;
	MVFinestData *data;
	d.super = vsapi->propGetNode(in, "super", 0, 0);
	d.vi = *vsapi->getVideoInfo(d.super);
	if (!isConstantFormat(&d.vi) || d.vi.format->bitsPerSample < 32 || d.vi.format->sampleType != stFloat) {
		vsapi->setError(out, "Finest: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.super);
		return;
	}
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Finest: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.super);
		return;
	}
	const VSMap *props = vsapi->getFramePropsRO(evil);
	int32_t evil_err[6];
	d.nHeight = int64ToIntS(vsapi->propGetInt(props, "Super_height", 0, &evil_err[0]));
	d.nSuperHPad = int64ToIntS(vsapi->propGetInt(props, "Super_hpad", 0, &evil_err[1]));
	d.nSuperVPad = int64ToIntS(vsapi->propGetInt(props, "Super_vpad", 0, &evil_err[2]));
	d.nSuperPel = int64ToIntS(vsapi->propGetInt(props, "Super_pel", 0, &evil_err[3]));
	d.nSuperModeYUV = int64ToIntS(vsapi->propGetInt(props, "Super_modeyuv", 0, &evil_err[4]));
	d.nSuperLevels = int64ToIntS(vsapi->propGetInt(props, "Super_levels", 0, &evil_err[5]));
	vsapi->freeFrame(evil);
	for (int32_t i = 0; i < 6; i++)
		if (evil_err[i]) {
			vsapi->setError(out, "Finest: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.super);
			return;
		}
	d.nPel = d.nSuperPel;
	int32_t nSuperWidth = d.vi.width;
	d.nWidth = nSuperWidth - 2 * d.nSuperHPad;
	d.xRatioUV = 1 << d.vi.format->subSamplingW;
	d.yRatioUV = 1 << d.vi.format->subSamplingH;
	d.vi.width = (d.nWidth + 2 * d.nSuperHPad) * d.nSuperPel;
	d.vi.height = (d.nHeight + 2 * d.nSuperVPad) * d.nSuperPel;
	data = new MVFinestData;
	*data = d;
	vsapi->createFilter(in, out, "Finest", mvfinestInit, mvfinestGetFrame, mvfinestFree, fmParallel, 0, data, core);
}

void mvfinestRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Finest",
		"super:clip;"
		, mvfinestCreate, 0, plugin);
}
