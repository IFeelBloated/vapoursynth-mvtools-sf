#include "VapourSynth.h"
#include "VSHelper.h"
#include "MVClip.h"
#include "MVFilter.hpp"

struct MVSCDetectionData {
	VSNodeRef *node;
	const VSVideoInfo *vi;
	VSNodeRef *vectors;
	double thscd1;
	double thscd2;
	MVFilter *bleh;
	MVClipDicks *mvClip;
};

static void VS_CC mvscdetectionInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVSCDetectionData *d = reinterpret_cast<MVSCDetectionData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvscdetectionGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVSCDetectionData *d = reinterpret_cast<MVSCDetectionData *>(*instanceData);
	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->vectors, frameCtx);
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		VSFrameRef *dst = vsapi->copyFrame(src, core);
		vsapi->freeFrame(src);
		const VSFrameRef *mvn = vsapi->getFrameFilter(n, d->vectors, frameCtx);
		MVClipBalls balls(d->mvClip, vsapi);
		balls.Update(mvn);
		vsapi->freeFrame(mvn);
		const char *propNames[2] = { "_SceneChangePrev", "_SceneChangeNext" };
		VSMap *props = vsapi->getFramePropsRW(dst);
		vsapi->propSetInt(props, propNames[(int32_t)d->mvClip->IsBackward()], !balls.IsUsable(), paReplace);
		return dst;
	}
	return nullptr;
}

static void VS_CC mvscdetectionFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVSCDetectionData *d = reinterpret_cast<MVSCDetectionData *>(instanceData);
	vsapi->freeNode(d->node);
	vsapi->freeNode(d->vectors);
	delete d->mvClip;
	delete d->bleh;
	delete d;
}

static void VS_CC mvscdetectionCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVSCDetectionData d;
	MVSCDetectionData *data;
	int err;
	d.thscd1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.thscd1 = MV_DEFAULT_SCD1;
	d.thscd2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.thscd2 = MV_DEFAULT_SCD2;
	d.vectors = vsapi->propGetNode(in, "vectors", 0, nullptr);
	try {
		d.mvClip = new MVClipDicks(d.vectors, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("SCDetection: ").append(e.what()).c_str());
		vsapi->freeNode(d.vectors);
		return;
	}
	try {
		d.bleh = new MVFilter(d.vectors, "SCDetection", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("SCDetection: ").append(e.what()).c_str());
		vsapi->freeNode(d.vectors);
		delete d.mvClip;
		return;
	}
	d.node = vsapi->propGetNode(in, "clip", 0, nullptr);
	d.vi = vsapi->getVideoInfo(d.node);
	data = new MVSCDetectionData;
	*data = d;
	vsapi->createFilter(in, out, "SCDetection", mvscdetectionInit, mvscdetectionGetFrame, mvscdetectionFree, fmParallel, 0, data, core);
}

void mvscdetectionRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("SCDetection",
		"clip:clip;"
		"vectors:clip;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvscdetectionCreate, 0, plugin);
}
