#pragma once
#include "VapourSynth.h"
#include "VSHelper.h"
#include "CommonFunctions.h"
#include "MaskFun.hpp"
#include "MVFilter.hpp"
#include "SimpleResize.hpp"

using FlowFunction = auto (*)(uint8_t *, int32_t, const uint8_t *, int32_t, int32_t *, int32_t, int32_t *, int32_t, int32_t, int32_t, int32_t, int32_t)->void;

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
		MakeVectorSmallMasks(balls, nBlkX, nBlkY, VXSmallY, nBlkXP, VYSmallY, nBlkXP);
		CheckAndPadSmallY(VXSmallY, VYSmallY, nBlkXP, nBlkYP, nBlkX, nBlkY);
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
		d->upsizer->Resize(VXFullY, VPitchY, VXSmallY, nBlkXP, true);
		d->upsizer->Resize(VYFullY, VPitchY, VYSmallY, nBlkXP, false);
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
			d->upsizerUV->Resize(VXFullUV, VPitchUV, VXSmallUV, nBlkXP, true);
			d->upsizerUV->Resize(VYFullUV, VPitchUV, VYSmallUV, nBlkXP, false);
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

auto CreateFlow(auto in, auto out, auto core, auto vsapi) {
	MVFlowData d;
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
		return d;
	}
	if (static_cast<FlowModes>(d.mode) < FlowModes::Fetch || static_cast<FlowModes>(d.mode) > FlowModes::Shift) {
		vsapi->setError(out, "Flow: mode must be 0 or 1.");
		return d;
	}
	d.time256 = static_cast<int32_t>(time * 256. / 100.);
	d.super = vsapi->propGetNode(in, "super", 0, nullptr);
	char errorMsg[1024];
	auto evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Flow: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.super);
		return d;
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
			return d;
		}
	d.vectors = vsapi->propGetNode(in, "vectors", 0, nullptr);
	try {
		d.mvClip = new MVClipDicks(d.vectors, d.thscd1, d.thscd2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Flow: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		return d;
	}
	try {
		d.bleh = new MVFilter(d.vectors, "Flow", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Flow: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		delete d.mvClip;
		return d;
	}
	if (d.bleh->nPel == 1)
		d.finest = vsapi->cloneNodeRef(d.super);
	else
	{
		auto mvtoolsPlugin = vsapi->getPluginById("com.zonked.mvsf", core);
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
			return d;
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
			return d;
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
		return d;
	}
	if (!isConstantFormat(d.vi) || d.vi->format->bitsPerSample < 32 || d.vi->format->sampleType != stFloat) {
		vsapi->setError(out, "Flow: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.finest);
		vsapi->freeNode(d.vectors);
		vsapi->freeNode(d.clip);
		delete d.mvClip;
		delete d.bleh;
		return d;
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
	d.upsizer = new SimpleResize<int32_t>(d.bleh->nWidth, d.bleh->nHeight, d.bleh->nBlkX, d.bleh->nBlkY, d.bleh->nWidth, d.bleh->nHeight, d.bleh->nPel);
	if (d.vi->format->colorFamily != cmGray)
		d.upsizerUV = new SimpleResize<int32_t>(d.nWidthUV, d.nHeightUV, d.bleh->nBlkX, d.bleh->nBlkY, d.nWidthUV, d.nHeightUV, d.bleh->nPel);
	if (static_cast<FlowModes>(d.mode) == FlowModes::Fetch)
		d.flow_function = flowFetch;
	else if (static_cast<FlowModes>(d.mode) == FlowModes::Shift)
		d.flow_function = flowShift;
	return d;
}

static auto VS_CC mvflowCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	auto args = ArgumentList{ in };
	auto Core = VaporCore{ core };
	auto clip = static_cast<Clip>(args["clip"]);
	auto vectors = static_cast<Clip>(args["vectors"]);
	auto cclip = Clip{};
	if (args["cclip"].Exists())
		cclip = args["cclip"];
	else
		cclip = clip;
	auto Eval = [&](auto&& vec) {
		auto CreateArgumentMap = [&]() {
			auto Map = vsapi->createMap();
			for (auto x : Range{ vsapi->propNumKeys(in) }) {
				auto ItemSrc = ReadonlyItem{ in, vsapi->propGetKey(in, x) };
				auto ItemDst = WritableItem{ Map, vsapi->propGetKey(in, x) };
				if (ItemSrc.Type() == VSPropTypes::ptNode)
					ItemDst = static_cast<Clip>(ItemSrc);
				else if (ItemSrc.Type() == VSPropTypes::ptInt)
					ItemDst = static_cast<std::int64_t>(ItemSrc);
				else if (ItemSrc.Type() == VSPropTypes::ptFloat)
					ItemDst = static_cast<double>(ItemSrc);
			}
			auto v = WritableItem{ Map, "vectors" };
			v.Erase();
			v = vec;
			return Map;
		};
		auto argMap = CreateArgumentMap();
		auto evalMap = vsapi->createMap();
		auto data = new MVFlowData{ CreateFlow(argMap, out, core, vsapi) };
		if (vsapi->getError(out) != nullptr) {
			delete data;
			vsapi->freeMap(argMap);
			vsapi->freeMap(evalMap);
			return Clip{};
		}
		vsapi->createFilter(argMap, evalMap, "Flow", mvflowInit, mvflowGetFrame, mvflowFree, fmParallel, 0, data, core);
		auto flow = vsapi->propGetNode(evalMap, "clip", 0, nullptr);
		vsapi->freeMap(argMap);
		vsapi->freeMap(evalMap);
		return Clip{ flow };
	};
	if (clip.FrameCount == vectors.FrameCount) {
		auto data = new MVFlowData{ CreateFlow(in, out, core, vsapi) };
		if (vsapi->getError(out) != nullptr) {
			delete data;
			return;
		}
		vsapi->createFilter(in, out, "Flow", mvflowInit, mvflowGetFrame, mvflowFree, fmParallel, 0, data, core);
	}
	else {
		auto radius = vectors.FrameCount / clip.FrameCount / 2;
		auto flows = std::vector<Clip>{};
		flows.reserve(2 * radius + 1);
		for (auto x : Range{ radius }) {
			auto flow = Eval(Core["std"]["SelectEvery"]("clip", vectors, "cycle", 2 * radius, "offsets", x));
			if (flow.ContainsVideoReference() == false)
				return;
			flows.push_back(std::move(flow));
		}
		flows.push_back(cclip);
		for (auto x : Range{ radius, 2 * radius })
			flows.push_back(Eval(Core["std"]["SelectEvery"]("clip", vectors, "cycle", 2 * radius, "offsets", x)));
		auto flowmulti = Core["std"]["Interleave"]("clips", flows);
		VaporGlobals::API->propSetNode(out, "clip", flowmulti.VideoNode, VSPropAppendMode::paAppend);
	}
}

auto mvflowRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Flow",
		"clip:clip;"
		"super:clip;"
		"vectors:clip;"
		"cclip:clip:opt;"
		"time:float:opt;"
		"mode:int:opt;"
		"fields:int:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		"tff:int:opt;"
		, mvflowCreate, 0, plugin);
}
