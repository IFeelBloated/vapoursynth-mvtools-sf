#include "VapourSynth.h"
#include "VSHelper.h"
#include "CommonFunctions.h"
#include "MaskFun.h"
#include "MVFilter.h"
#include "SimpleResize.hpp"

typedef auto (*FlowFunction)(uint8_t *, int32_t, const uint8_t *, int32_t, int32_t *, int32_t, int32_t *, int32_t, int32_t, int32_t, int32_t, int32_t)->void;

enum class FlowModes {
	Fetch = 0,
	Shift
};

struct MVFlowData final {
	VSNodeRef *clip;
	const VSVideoInfo *vi;
	VSNodeRef *finest;
	VSNodeRef *super;
	VSNodeRef *vectors;
	int32_t time256;
	int32_t mode;
	int32_t fields;
	double thscd1;
	double thscd2;
	int32_t tff;
	int32_t tff_exists;
	MVClipDicks *mvClip;
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
	SimpleResize<int32_t> *upsizer;
	SimpleResize<int32_t> *upsizerUV;
	FlowFunction flow_function;
};

static auto VS_CC mvflowInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	auto d = reinterpret_cast<MVFlowData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

static auto flowFetch(uint8_t *pdst8, int32_t dst_pitch, const uint8_t *pref8, int32_t ref_pitch, int32_t *VXFull, int32_t VXPitch, int32_t *VYFull, int32_t VYPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel) {
	auto pref = reinterpret_cast<const float *>(pref8);
	auto pdst = reinterpret_cast<float *>(pdst8);
	dst_pitch /= sizeof(float);
	ref_pitch /= sizeof(float);
	auto nPelLog = ilog2(nPel);
	for (auto h = 0; h < height; ++h) {
		for (auto w = 0; w < width; ++w) {
			auto vx = (VXFull[w] * static_cast<int64_t>(time256) + 128) >> 8;
			auto vy = (VYFull[w] * static_cast<int64_t>(time256) + 128) >> 8;
			pdst[w] = pref[vy * ref_pitch + vx + (w << nPelLog)];
		}
		pref += ref_pitch << nPelLog;
		pdst += dst_pitch;
		VXFull += VXPitch;
		VYFull += VYPitch;
	}
}

static auto flowShift(uint8_t *pdst8, int32_t dst_pitch, const uint8_t *pref8, int32_t ref_pitch, int32_t *VXFull, int32_t VXPitch, int32_t *VYFull, int32_t VYPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel) {
	auto pref = reinterpret_cast<const float *>(pref8);
	auto pdst = reinterpret_cast<float *>(pdst8);
	dst_pitch /= sizeof(float);
	ref_pitch /= sizeof(float);
	auto nPelLog = ilog2(nPel);
	auto rounding = 128 << nPelLog;
	auto shift = 8 + nPelLog;
	for (auto h = 0; h < height; ++h) {
		for (auto w = 0; w < width; ++w) {
			auto vx = (-VXFull[w] * static_cast<int64_t>(time256) + rounding) >> shift;
			auto vy = (-VYFull[w] * static_cast<int64_t>(time256) + rounding) >> shift;
			auto href = h + vy;
			auto wref = w + vx;
			if (href >= 0 && href < height && wref >= 0 && wref < width)
				pdst[vy * dst_pitch + vx + w] = pref[w << nPelLog];
		}
		pref += ref_pitch << nPelLog;
		pdst += dst_pitch;
		VXFull += VXPitch;
		VYFull += VYPitch;
	}
}

static auto flowMemset(uint8_t *ptrv, float value, size_t bytes) {
	auto num = bytes / sizeof(float);
	auto ptr = reinterpret_cast<float *>(ptrv);
	while (num-- > 0)
		*ptr++ = value;
	return ptrv;
}

static auto VS_CC mvflowGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)->const VSFrameRef *{
	auto d = reinterpret_cast<const MVFlowData *>(*instanceData);
if (activationReason == arInitial) {
	auto offset = d->mvClip->GetDeltaFrame();
	decltype(offset) nref;
	if (offset > 0)
		nref = d->mvClip->IsBackward() ? n + offset : n - offset;
	else
		nref = -offset;
	vsapi->requestFrameFilter(n, d->vectors, frameCtx);
	if (nref >= 0 && nref < d->vi->numFrames) {
		if (n < nref) {
			vsapi->requestFrameFilter(n, d->clip, frameCtx);
			vsapi->requestFrameFilter(n, d->finest, frameCtx);
			vsapi->requestFrameFilter(nref, d->finest, frameCtx);
		}
		else {
			vsapi->requestFrameFilter(nref, d->finest, frameCtx);
			vsapi->requestFrameFilter(n, d->finest, frameCtx);
			vsapi->requestFrameFilter(n, d->clip, frameCtx);
		}
	}
	vsapi->requestFrameFilter(n, d->clip, frameCtx);
}
else if (activationReason == arAllFramesReady) {
	auto offset = d->mvClip->GetDeltaFrame();
	decltype(offset) nref;
	if (offset > 0)
		nref = d->mvClip->IsBackward() ? n + offset : n - offset;
	else
		nref = -offset;
	uint8_t *pDst[3];
	const uint8_t *pRef[3];
	int32_t nDstPitches[3];
	int32_t nRefPitches[3];
	auto mvn = vsapi->getFrameFilter(n, d->vectors, frameCtx);
	MVClipBalls balls(d->mvClip, vsapi);
	balls.Update(mvn);
	vsapi->freeFrame(mvn);
	if (balls.IsUsable()) {
		auto ref = vsapi->getFrameFilter(nref, d->finest, frameCtx);
		auto dst = vsapi->newVideoFrame(d->vi->format, d->vi->width, d->vi->height, ref, core);
		for (auto i = 0; i < d->vi->format->numPlanes; ++i) {
			pDst[i] = vsapi->getWritePtr(dst, i);
			pRef[i] = vsapi->getReadPtr(ref, i);
			nDstPitches[i] = vsapi->getStride(dst, i);
			nRefPitches[i] = vsapi->getStride(ref, i);
		}
		const auto nWidth = d->bleh->nWidth;
		const auto nHeight = d->bleh->nHeight;
		const auto nWidthUV = d->nWidthUV;
		const auto nHeightUV = d->nHeightUV;
		const auto nHeightP = d->nHeightP;
		const auto nHeightPUV = d->nHeightPUV;
		const auto xRatioUV = d->bleh->xRatioUV;
		const auto yRatioUV = d->bleh->yRatioUV;
		const auto nBlkX = d->bleh->nBlkX;
		const auto nBlkY = d->bleh->nBlkY;
		const auto nBlkXP = d->nBlkXP;
		const auto nBlkYP = d->nBlkYP;
		const auto nHPadding = d->bleh->nHPadding;
		const auto nVPadding = d->bleh->nVPadding;
		const auto nVPaddingUV = d->nVPaddingUV;
		const auto nHPaddingUV = d->nHPaddingUV;
		const auto nPel = d->bleh->nPel;
		const auto time256 = d->time256;
		const auto VPitchY = d->VPitchY;
		const auto VPitchUV = d->VPitchUV;
		auto bytesPerSample = d->vi->format->bytesPerSample;
		auto VXFullY = new int32_t[nHeightP * VPitchY];
		auto VYFullY = new int32_t[nHeightP * VPitchY];
		auto VXSmallY = new int32_t[nBlkYP * nBlkXP];
		auto VYSmallY = new int32_t[nBlkYP * nBlkXP];
		MakeVectorSmallMasks(&balls, nBlkX, nBlkY, VXSmallY, nBlkXP, VYSmallY, nBlkXP);
		if (nBlkXP > nBlkX)
			for (auto j = 0; j < nBlkY; ++j) {
				VXSmallY[j * nBlkXP + nBlkX] = std::min(VXSmallY[j * nBlkXP + nBlkX - 1], 0);
				VYSmallY[j * nBlkXP + nBlkX] = VYSmallY[j * nBlkXP + nBlkX - 1];
			}
		if (nBlkYP > nBlkY)
			for (auto i = 0; i < nBlkXP; ++i) {
				VXSmallY[nBlkXP * nBlkY + i] = VXSmallY[nBlkXP * (nBlkY - 1) + i];
				VYSmallY[nBlkXP * nBlkY + i] = std::min(VYSmallY[nBlkXP * (nBlkY - 1) + i], 0);
			}
		auto fieldShift = 0;
		if (d->fields && nPel > 1 && ((nref - n) % 2 != 0)) {
			auto src = vsapi->getFrameFilter(n, d->finest, frameCtx);
			int err;
			auto props = vsapi->getFramePropsRO(src);
			auto src_top_field = !!vsapi->propGetInt(props, "_Field", 0, &err);
			vsapi->freeFrame(src);
			if (err && !d->tff_exists) {
				vsapi->setFilterError("Flow: _Field property not found in super frame. Therefore, you must pass tff argument.", frameCtx);
				vsapi->freeFrame(dst);
				vsapi->freeFrame(ref);
				return nullptr;
			}
			if (d->tff_exists)
				src_top_field = !!(static_cast<int>(d->tff) ^ (n % 2));
			props = vsapi->getFramePropsRO(ref);
			auto ref_top_field = !!vsapi->propGetInt(props, "_Field", 0, &err);
			if (err && !d->tff_exists) {
				vsapi->setFilterError("Flow: _Field property not found in super frame. Therefore, you must pass tff argument.", frameCtx);
				vsapi->freeFrame(dst);
				vsapi->freeFrame(ref);
				return nullptr;
			}
			if (d->tff_exists)
				ref_top_field = !!(static_cast<int>(d->tff) ^ (nref % 2));
			fieldShift = (src_top_field && !ref_top_field) ? nPel / 2 : ((ref_top_field && !src_top_field) ? -(nPel / 2) : 0);
		}
		for (auto j = 0; j < nBlkYP; ++j)
			for (auto i = 0; i < nBlkXP; ++i)
				VYSmallY[j * nBlkXP + i] += fieldShift;
		d->upsizer->Resize(VXFullY, VPitchY, VXSmallY, nBlkXP);
		d->upsizer->Resize(VYFullY, VPitchY, VYSmallY, nBlkXP);
		auto nOffsetY = nRefPitches[0] * nVPadding * nPel + nHPadding * bytesPerSample * nPel;
		auto nOffsetUV = nRefPitches[1] * nVPaddingUV * nPel + nHPaddingUV * bytesPerSample * nPel;
		if (static_cast<FlowModes>(d->mode) == FlowModes::Shift)
			flowMemset(pDst[0], 1.f, nHeight * nDstPitches[0]);
		d->flow_function(pDst[0], nDstPitches[0], pRef[0] + nOffsetY, nRefPitches[0],
			VXFullY, VPitchY, VYFullY, VPitchY,
			nWidth, nHeight, time256, nPel);
		if (d->vi->format->colorFamily != cmGray) {
			auto VXFullUV = new int32_t[nHeightPUV * VPitchUV];
			auto VYFullUV = new int32_t[nHeightPUV * VPitchUV];
			auto VXSmallUV = new int32_t[nBlkYP * nBlkXP];
			auto VYSmallUV = new int32_t[nBlkYP * nBlkXP];
			VectorSmallMaskYToHalfUV(VXSmallY, nBlkXP, nBlkYP, VXSmallUV, xRatioUV);
			VectorSmallMaskYToHalfUV(VYSmallY, nBlkXP, nBlkYP, VYSmallUV, yRatioUV);
			d->upsizerUV->Resize(VXFullUV, VPitchUV, VXSmallUV, nBlkXP);
			d->upsizerUV->Resize(VYFullUV, VPitchUV, VYSmallUV, nBlkXP);
			if (static_cast<FlowModes>(d->mode) == FlowModes::Shift) {
				if (d->vi->format->colorFamily == cmRGB) {
					flowMemset(pDst[1], 1.f, nHeightUV * nDstPitches[1]);
					flowMemset(pDst[2], 1.f, nHeightUV * nDstPitches[2]);
				}
				else {
					flowMemset(pDst[1], 0.f, nHeightUV * nDstPitches[1]);
					flowMemset(pDst[2], 0.f, nHeightUV * nDstPitches[2]);
				}
			}
			d->flow_function(pDst[1], nDstPitches[1], pRef[1] + nOffsetUV, nRefPitches[1],
				VXFullUV, VPitchUV, VYFullUV, VPitchUV,
				nWidthUV, nHeightUV, time256, nPel);
			d->flow_function(pDst[2], nDstPitches[2], pRef[2] + nOffsetUV, nRefPitches[2],
				VXFullUV, VPitchUV, VYFullUV, VPitchUV,
				nWidthUV, nHeightUV, time256, nPel);
			delete[] VXFullUV;
			delete[] VYFullUV;
			delete[] VXSmallUV;
			delete[] VYSmallUV;
		}
		delete[] VXFullY;
		delete[] VYFullY;
		delete[] VXSmallY;
		delete[] VYSmallY;
		vsapi->freeFrame(ref);
		return dst;
	}
	else
		return vsapi->getFrameFilter(n, d->clip, frameCtx);
}
return nullptr;
}

static auto VS_CC mvflowFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	auto d = reinterpret_cast<MVFlowData *>(instanceData);
	delete d->mvClip;
	delete d->bleh;
	delete d->upsizer;
	if (d->vi->format->colorFamily != cmGray)
		delete d->upsizerUV;
	vsapi->freeNode(d->finest);
	vsapi->freeNode(d->super);
	vsapi->freeNode(d->vectors);
	vsapi->freeNode(d->clip);
	delete d;
}

static auto VS_CC mvflowCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVFlowData d;
	MVFlowData *data;
	int err;
	auto time = vsapi->propGetFloat(in, "time", 0, &err);
	if (err)
		time = 100.;
	d.mode = int64ToIntS(vsapi->propGetInt(in, "mode", 0, &err));
	d.fields = !!vsapi->propGetInt(in, "fields", 0, &err);
	d.thscd1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.thscd1 = MV_DEFAULT_SCD1;
	d.thscd2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.thscd2 = MV_DEFAULT_SCD2;
	d.tff = !!vsapi->propGetInt(in, "tff", 0, &err);
	d.tff_exists = !err;
	if (time < 0.0 || time > 100.0) {
		vsapi->setError(out, "Flow: time must be between 0 and 100 % (inclusive).");
		return;
	}
	if (static_cast<FlowModes>(d.mode) < FlowModes::Fetch || static_cast<FlowModes>(d.mode) > FlowModes::Shift) {
		vsapi->setError(out, "Flow: mode must be 0 or 1.");
		return;
	}
	d.time256 = static_cast<int32_t>(time * 256. / 100.);
	d.super = vsapi->propGetNode(in, "super", 0, nullptr);
	char errorMsg[1024];
	auto evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Flow: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.super);
		return;
	}
	auto props = vsapi->getFramePropsRO(evil);
	int evil_err[2];
	auto nHeightS = int64ToIntS(vsapi->propGetInt(props, "Super_height", 0, &evil_err[0]));
	d.nSuperHPad = int64ToIntS(vsapi->propGetInt(props, "Super_hpad", 0, &evil_err[1]));
	vsapi->freeFrame(evil);
	for (auto i = 0; i < 2; ++i)
		if (evil_err[i]) {
			vsapi->setError(out, "Flow: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.super);
			return;
		}
	d.vectors = vsapi->propGetNode(in, "vectors", 0, nullptr);
	try {
		d.mvClip = new MVClipDicks(d.vectors, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Flow: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		return;
	}
	try {
		d.bleh = new MVFilter(d.vectors, "Flow", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Flow: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		delete d.mvClip;
		return;
	}
	if (d.bleh->nPel == 1)
		d.finest = vsapi->cloneNodeRef(d.super);
	else
	{
		auto mvtoolsPlugin = vsapi->getPluginById("com.nodame.mvsf", core);
		auto stdPlugin = vsapi->getPluginById("com.vapoursynth.std", core);
		auto args = vsapi->createMap();
		vsapi->propSetNode(args, "super", d.super, paReplace);
		auto ret = vsapi->invoke(mvtoolsPlugin, "Finest", args);
		if (vsapi->getError(ret)) {
			vsapi->setError(out, std::string("Flow: ").append(vsapi->getError(ret)).c_str());
			delete d.mvClip;
			delete d.bleh;
			vsapi->freeNode(d.super);
			vsapi->freeNode(d.vectors);
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
			vsapi->setError(out, std::string("FlowBlur: ").append(vsapi->getError(ret)).c_str());
			delete d.mvClip;
			delete d.bleh;
			vsapi->freeNode(d.super);
			vsapi->freeNode(d.vectors);
			vsapi->freeMap(ret);
			return;
		}
		d.finest = vsapi->propGetNode(ret, "clip", 0, nullptr);
		vsapi->freeMap(ret);
	}
	d.clip = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = vsapi->getVideoInfo(d.clip);
	auto supervi = vsapi->getVideoInfo(d.super);
	auto nSuperWidth = supervi->width;
	if (d.bleh->nHeight != nHeightS || d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2) {
		vsapi->setError(out, "Flow: wrong source or super clip frame size.");
		vsapi->freeNode(d.finest);
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		vsapi->freeNode(d.clip);
		delete d.mvClip;
		delete d.bleh;
		return;
	}
	if (!isConstantFormat(d.vi) || d.vi->format->bitsPerSample < 32 || d.vi->format->sampleType != stFloat) {
		vsapi->setError(out, "Flow: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.finest);
		vsapi->freeNode(d.vectors);
		vsapi->freeNode(d.clip);
		delete d.mvClip;
		delete d.bleh;
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
	d.upsizer = new SimpleResize<int32_t>(d.bleh->nWidth, d.bleh->nHeight, d.bleh->nBlkX, d.bleh->nBlkY);
	if (d.vi->format->colorFamily != cmGray)
		d.upsizerUV = new SimpleResize<int32_t>(d.nWidthUV, d.nHeightUV, d.bleh->nBlkX, d.bleh->nBlkY);
	if (static_cast<FlowModes>(d.mode) == FlowModes::Fetch)
		d.flow_function = flowFetch;
	else if (static_cast<FlowModes>(d.mode) == FlowModes::Shift)
		d.flow_function = flowShift;
	data = new MVFlowData;
	*data = d;
	vsapi->createFilter(in, out, "Flow", mvflowInit, mvflowGetFrame, mvflowFree, fmParallel, 0, data, core);
}

auto mvflowRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Flow",
		"clip:clip;"
		"super:clip;"
		"vectors:clip;"
		"time:float:opt;"
		"mode:int:opt;"
		"fields:int:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		"tff:int:opt;"
		, mvflowCreate, 0, plugin);
}
