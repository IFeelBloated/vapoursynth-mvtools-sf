#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "Include/VSHelper.h"
#include "DCTFFTW.hpp"
#include "GroupOfPlanes.h"
#include "MVInterface.h"

struct MVRecalculateData {
	VSNodeRef *node;
	const VSVideoInfo *vi;
	const VSVideoInfo *supervi;
	MVClipDicks *mvClip;
	MVAnalysisData analysisData;
	MVAnalysisData analysisDataDivided;
	double nLambda;
	SearchType searchType;
	int32_t nSearchParam;
	int32_t pnew;
	int32_t plen;
	int32_t divideExtra;
	bool meander;
	int32_t dctmode;
	int32_t nModeYUV;
	int32_t headerSize;
	int32_t nSuperLevels;
	int32_t nSuperHPad;
	int32_t nSuperVPad;
	int32_t nSuperPel;
	int32_t nSuperModeYUV;
	int32_t blksize;
	int32_t blksizev;
	int32_t search;
	int32_t searchparam;
	bool chroma;
	bool truemotion;
	int32_t overlap;
	int32_t overlapv;
	int32_t smooth;
	double thSAD;
	VSNodeRef *vectors;
	bool fields;
	bool tff;
	int32_t tffexists;
};

static void VS_CC mvrecalculateInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVRecalculateData *d = reinterpret_cast<MVRecalculateData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvrecalculateGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVRecalculateData *d = reinterpret_cast<MVRecalculateData *>(*instanceData);
	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->vectors, frameCtx);
		int32_t offset = (d->analysisData.isBackward) ? d->analysisData.nDeltaFrame : -d->analysisData.nDeltaFrame;
		int32_t nref = n + offset;
		if (nref >= 0 && (nref < d->vi->numFrames || !d->vi->numFrames)) {
			if (n < nref) {
				vsapi->requestFrameFilter(n, d->node, frameCtx);
				vsapi->requestFrameFilter(nref, d->node, frameCtx);
			}
			else {
				vsapi->requestFrameFilter(nref, d->node, frameCtx);
				vsapi->requestFrameFilter(n, d->node, frameCtx);
			}
		}
		else {
			vsapi->requestFrameFilter(n, d->node, frameCtx);
		}
	}
	else if (activationReason == arAllFramesReady) {
		GroupOfPlanes *vectorFields = new GroupOfPlanes(d->analysisData.nBlkSizeX, d->analysisData.nBlkSizeY, d->analysisData.nLvCount, d->analysisData.nPel, d->analysisData.nMotionFlags, d->analysisData.nOverlapX, d->analysisData.nOverlapY, d->analysisData.nBlkX, d->analysisData.nBlkY, d->analysisData.xRatioUV, d->analysisData.yRatioUV, d->divideExtra);
		const uint8_t *pSrc[3] = { nullptr };
		const uint8_t *pRef[3] = { nullptr };
		uint8_t *pDst = { nullptr };
		int32_t nSrcPitch[3] = { 0 };
		int32_t nRefPitch[3] = { 0 };
		int32_t offset = (d->analysisData.isBackward) ? d->analysisData.nDeltaFrame : -d->analysisData.nDeltaFrame;
		int32_t nref = n + offset;
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		const VSMap *srcprops = vsapi->getFramePropsRO(src);
		int err;
		bool srctff = !!vsapi->propGetInt(srcprops, "_Field", 0, &err);
		if (err && d->fields && !d->tffexists) {
			vsapi->setFilterError("Recalculate: _Field property not found in input frame. Therefore, you must pass tff argument.", frameCtx);
			delete vectorFields;
			vsapi->freeFrame(src);
			return nullptr;
		}
		if (d->tffexists)
			srctff = !!(static_cast<int>(d->tff) ^ (n % 2));
		for (int32_t plane = 0; plane < d->supervi->format->numPlanes; plane++) {
			pSrc[plane] = vsapi->getReadPtr(src, plane);
			nSrcPitch[plane] = vsapi->getStride(src, plane);
		}
		int32_t dst_height = 1;
		int32_t dst_width = d->headerSize / sizeof(int32_t) + vectorFields->GetArraySize();
		dst_width *= 4;
		VSFrameRef *dst = vsapi->newVideoFrame(d->vi->format, dst_width, dst_height, src, core);
		pDst = vsapi->getWritePtr(dst, 0);
		memcpy(pDst, &d->headerSize, sizeof(int32_t));
		if (d->divideExtra)
			memcpy(pDst + sizeof(int32_t), &d->analysisDataDivided, sizeof(d->analysisData));
		else
			memcpy(pDst + sizeof(int32_t), &d->analysisData, sizeof(d->analysisData));
		pDst += d->headerSize;
		const VSFrameRef *mvn = vsapi->getFrameFilter(n, d->vectors, frameCtx);
		MVClipBalls balls(d->mvClip, vsapi);
		balls.Update(mvn);
		vsapi->freeFrame(mvn);
		if (balls.IsUsable() && nref >= 0 && (nref < d->vi->numFrames || !d->vi->numFrames)) {
			const VSFrameRef *ref = vsapi->getFrameFilter(nref, d->node, frameCtx);
			const VSMap *refprops = vsapi->getFramePropsRO(ref);
			bool reftff = !!vsapi->propGetInt(refprops, "_Field", 0, &err);
			if (err && d->fields && !d->tffexists) {
				vsapi->setFilterError("Recalculate: _Field property not found in input frame. Therefore, you must pass tff argument.", frameCtx);
				delete vectorFields;
				vsapi->freeFrame(src);
				vsapi->freeFrame(ref);
				vsapi->freeFrame(dst);
				return nullptr;
			}
			if (d->tffexists)
				reftff = !!(static_cast<int>(d->tff) ^ (nref % 2));
			int32_t fieldShift = 0;
			if (d->fields && d->analysisData.nPel > 1 && (d->analysisData.nDeltaFrame % 2)) {
				fieldShift = (srctff && !reftff) ? d->analysisData.nPel / 2 : ((reftff && !srctff) ? -(d->analysisData.nPel / 2) : 0);
			}
			for (int32_t plane = 0; plane < d->supervi->format->numPlanes; ++plane) {
				pRef[plane] = vsapi->getReadPtr(ref, plane);
				nRefPitch[plane] = vsapi->getStride(ref, plane);
			}
			MVGroupOfFrames *pSrcGOF = new MVGroupOfFrames(d->nSuperLevels, d->analysisData.nWidth, d->analysisData.nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, d->analysisData.xRatioUV, d->analysisData.yRatioUV);
			MVGroupOfFrames *pRefGOF = new MVGroupOfFrames(d->nSuperLevels, d->analysisData.nWidth, d->analysisData.nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, d->analysisData.xRatioUV, d->analysisData.yRatioUV);
			pSrcGOF->Update(d->nModeYUV, (uint8_t *)pSrc[0], nSrcPitch[0], (uint8_t *)pSrc[1], nSrcPitch[1], (uint8_t *)pSrc[2], nSrcPitch[2]); // v2.0
			pRefGOF->Update(d->nModeYUV, (uint8_t *)pRef[0], nRefPitch[0], (uint8_t *)pRef[1], nRefPitch[1], (uint8_t *)pRef[2], nRefPitch[2]); // v2.0
			DCTClass *DCTc = nullptr;
			if (d->dctmode != 0) {
				DCTc = new DCTFFTW(d->blksize, d->blksizev, d->dctmode);
			}
			vectorFields->RecalculateMVs(balls, pSrcGOF, pRefGOF, d->searchType, d->nSearchParam, d->nLambda, d->pnew, reinterpret_cast<int32_t*>(pDst), nullptr, fieldShift, d->thSAD, DCTc, d->smooth, d->meander);
			if (d->divideExtra) {
				vectorFields->ExtraDivide(reinterpret_cast<int32_t*>(pDst));
			}
			delete vectorFields;
			if (DCTc)
				delete DCTc;
			delete pSrcGOF;
			delete pRefGOF;
			vsapi->freeFrame(ref);
		}
		else {
			vectorFields->WriteDefaultToArray(reinterpret_cast<int32_t*>(pDst));
			delete vectorFields;
		}
		vsapi->freeFrame(src);
		return dst;
	}
	return nullptr;
}

static void VS_CC mvrecalculateFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVRecalculateData *d = reinterpret_cast<MVRecalculateData *>(instanceData);
	vsapi->freeNode(d->node);
	vsapi->freeNode(d->vectors);
	delete d->mvClip;
	delete d;
}

auto CreateVector(auto in, auto out, auto vsapi) {
	MVRecalculateData d;
	int err;
	d.thSAD = vsapi->propGetFloat(in, "thsad", 0, &err);
	if (err)
		d.thSAD = 200.;
	d.smooth = int64ToIntS(vsapi->propGetInt(in, "smooth", 0, &err));
	if (err)
		d.smooth = 1;
	d.blksize = int64ToIntS(vsapi->propGetInt(in, "blksize", 0, &err));
	if (err)
		d.blksize = 8;
	d.blksizev = int64ToIntS(vsapi->propGetInt(in, "blksizev", 0, &err));
	if (err)
		d.blksizev = d.blksize;
	d.search = int64ToIntS(vsapi->propGetInt(in, "search", 0, &err));
	if (err)
		d.search = 4;
	d.searchparam = int64ToIntS(vsapi->propGetInt(in, "searchparam", 0, &err));
	if (err)
		d.searchparam = 2;
	d.chroma = !!vsapi->propGetInt(in, "chroma", 0, &err);
	if (err)
		d.chroma = 1;
	d.truemotion = !!vsapi->propGetInt(in, "truemotion", 0, &err);
	if (err)
		d.truemotion = 1;
	d.nLambda = vsapi->propGetFloat(in, "lambda", 0, &err);
	if (err)
		d.nLambda = d.truemotion ? (1000 * d.blksize * d.blksizev / 64) : 0.;
	d.pnew = int64ToIntS(vsapi->propGetInt(in, "pnew", 0, &err));
	if (err)
		d.pnew = d.truemotion ? 50 : 0;
	d.overlap = int64ToIntS(vsapi->propGetInt(in, "overlap", 0, &err));
	d.overlapv = int64ToIntS(vsapi->propGetInt(in, "overlapv", 0, &err));
	if (err)
		d.overlapv = d.overlap;
	d.dctmode = int64ToIntS(vsapi->propGetInt(in, "dct", 0, &err));
	d.divideExtra = int64ToIntS(vsapi->propGetInt(in, "divide", 0, &err));
	d.meander = !!vsapi->propGetInt(in, "meander", 0, &err);
	if (err)
		d.meander = 1;
	d.fields = !!vsapi->propGetInt(in, "fields", 0, &err);
	d.tff = !!vsapi->propGetInt(in, "tff", 0, &err);
	d.tffexists = !err;
	if (d.search < 0 || d.search > 7) {
		vsapi->setError(out, "Recalculate: search must be between 0 and 7 (inclusive).");
		return d;
	}
	if (d.dctmode < 0 || d.dctmode > 10) {
		vsapi->setError(out, "Recalculate: dct must be between 0 and 10 (inclusive).");
		return d;
	}
	if (d.dctmode >= 5 && !((d.blksize == 4 && d.blksizev == 4) ||
		(d.blksize == 8 && d.blksizev == 8) ||
		(d.blksize == 16 && d.blksizev == 16) ||
		(d.blksize == 32 && d.blksizev == 32) ||
		(d.blksize == 64 && d.blksizev == 64) ||
		(d.blksize == 128 && d.blksizev == 128) ||
		(d.blksize == 256 && d.blksizev == 256))) {
		vsapi->setError(out, "Recalculate: dct 5..10 can only work with 4x4, 8x8, 16x16, and 32x32 blocks.");
		return d;
	}
	if (d.divideExtra < 0 || d.divideExtra > 2) {
		vsapi->setError(out, "Recalculate: divide must be between 0 and 2 (inclusive).");
		return d;
	}
	d.analysisData.nBlkSizeX = d.blksize;
	d.analysisData.nBlkSizeY = d.blksizev;
	if ((d.analysisData.nBlkSizeX != 2 || d.analysisData.nBlkSizeY != 2) &&
		(d.analysisData.nBlkSizeX != 4 || d.analysisData.nBlkSizeY != 4) &&
		(d.analysisData.nBlkSizeX != 8 || d.analysisData.nBlkSizeY != 4) &&
		(d.analysisData.nBlkSizeX != 8 || d.analysisData.nBlkSizeY != 8) &&
		(d.analysisData.nBlkSizeX != 16 || d.analysisData.nBlkSizeY != 2) &&
		(d.analysisData.nBlkSizeX != 16 || d.analysisData.nBlkSizeY != 8) &&
		(d.analysisData.nBlkSizeX != 16 || d.analysisData.nBlkSizeY != 16) &&
		(d.analysisData.nBlkSizeX != 32 || d.analysisData.nBlkSizeY != 32) &&
		(d.analysisData.nBlkSizeX != 32 || d.analysisData.nBlkSizeY != 16) &&
		(d.analysisData.nBlkSizeX != 64 || d.analysisData.nBlkSizeY != 32) &&
		(d.analysisData.nBlkSizeX != 64 || d.analysisData.nBlkSizeY != 64) &&
		(d.analysisData.nBlkSizeX != 128 || d.analysisData.nBlkSizeY != 64) &&
		(d.analysisData.nBlkSizeX != 128 || d.analysisData.nBlkSizeY != 128) &&
		(d.analysisData.nBlkSizeX != 256 || d.analysisData.nBlkSizeY != 128) &&
		(d.analysisData.nBlkSizeX != 256 || d.analysisData.nBlkSizeY != 256)) {
		vsapi->setError(out, "Recalculate: the block size must be 2x2, 4x4, 8x4, 8x8, 16x2, 16x8, 16x16, 32x16, or 32x32.");
		return d;
	}
	if (d.pnew < 0 || d.pnew > 256) {
		vsapi->setError(out, "Recalculate: pnew must be between 0 and 256 (inclusive).");
		return d;
	}
	if (d.overlap < 0 || d.overlap > d.blksize / 2 ||
		d.overlapv < 0 || d.overlapv > d.blksizev / 2) {
		vsapi->setError(out, "Recalculate: overlap must be at most half of blksize, overlapv must be at most half of blksizev, and they both need to be at least 0.");
		return d;
	}
	if (d.divideExtra && (d.blksize < 8 || d.blksizev < 8)) {
		vsapi->setError(out, "Recalculate: blksize and blksizev must be at least 8 when divide=True.");
		return d;
	}
	d.analysisData.nOverlapX = d.overlap;
	d.analysisData.nOverlapY = d.overlapv;
	SearchType searchTypes[] = { ONETIME, NSTEP, LOGARITHMIC, EXHAUSTIVE, HEX2SEARCH, UMHSEARCH, HSEARCH, VSEARCH };
	d.searchType = searchTypes[d.search];
	if (d.searchType == NSTEP)
		d.nSearchParam = (d.searchparam < 0) ? 0 : d.searchparam;
	else
		d.nSearchParam = (d.searchparam < 1) ? 1 : d.searchparam;
	d.analysisData.nMagicKey = MotionMagicKey;
	d.analysisData.nVersion = MVAnalysisDataVersion;
	d.headerSize = VSMAX(4 + sizeof(d.analysisData), 256);
	d.node = vsapi->propGetNode(in, "super", 0, 0);
	d.supervi = vsapi->getVideoInfo(d.node);
	if (d.overlap % (1 << d.supervi->format->subSamplingW) ||
		d.overlapv % (1 << d.supervi->format->subSamplingH)) {
		vsapi->setError(out, "Recalculate: The requested overlap is incompatible with the super clip's subsampling.");
		vsapi->freeNode(d.node);
		return d;
	}
	if (d.divideExtra && (d.overlap % (2 << d.supervi->format->subSamplingW) ||
		d.overlapv % (2 << d.supervi->format->subSamplingH))) {
		vsapi->setError(out, "Recalculate: overlap and overlapv must be multiples of 2 or 4 when divide=True, depending on the super clip's subsampling.");
		vsapi->freeNode(d.node);
		return d;
	}
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.node, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Recalculate: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.node);
		return d;
	}
	const VSMap *props = vsapi->getFramePropsRO(evil);
	int32_t evil_err[6];
	int32_t nHeight = int64ToIntS(vsapi->propGetInt(props, "Super_height", 0, &evil_err[0]));
	d.nSuperHPad = int64ToIntS(vsapi->propGetInt(props, "Super_hpad", 0, &evil_err[1]));
	d.nSuperVPad = int64ToIntS(vsapi->propGetInt(props, "Super_vpad", 0, &evil_err[2]));
	d.nSuperPel = int64ToIntS(vsapi->propGetInt(props, "Super_pel", 0, &evil_err[3]));
	d.nSuperModeYUV = int64ToIntS(vsapi->propGetInt(props, "Super_modeyuv", 0, &evil_err[4]));
	d.nSuperLevels = int64ToIntS(vsapi->propGetInt(props, "Super_levels", 0, &evil_err[5]));
	vsapi->freeFrame(evil);
	for (int32_t i = 0; i < 6; i++)
		if (evil_err[i]) {
			vsapi->setError(out, "Recalculate: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.node);
			return d;
		}
	if (d.supervi->format->colorFamily == cmGray)
		d.chroma = 0;
	if (d.supervi->format->colorFamily == cmRGB)
		d.chroma = 1;
	d.nModeYUV = d.chroma ? YUVPLANES : YPLANE;
	if ((d.nModeYUV & d.nSuperModeYUV) != d.nModeYUV) { //x
		vsapi->setError(out, "Recalculate: super clip does not contain needed color data.");
		vsapi->freeNode(d.node);
		return d;
	}
	d.vectors = vsapi->propGetNode(in, "vectors", 0, nullptr);
	d.vi = vsapi->getVideoInfo(d.vectors);
	evil = vsapi->getFrame(0, d.vectors, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Recalculate: failed to retrieve first frame from vectors clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.node);
		vsapi->freeNode(d.vectors);
		return d;
	}
	const MVAnalysisData *pAnalyzeFilter = reinterpret_cast<const MVAnalysisData *>(vsapi->getReadPtr(evil, 0) + sizeof(int32_t));
	d.analysisData.yRatioUV = pAnalyzeFilter->GetYRatioUV();
	d.analysisData.xRatioUV = pAnalyzeFilter->GetXRatioUV();
	d.analysisData.nWidth = pAnalyzeFilter->GetWidth();
	d.analysisData.nHeight = pAnalyzeFilter->GetHeight();
	d.analysisData.nDeltaFrame = pAnalyzeFilter->GetDeltaFrame();
	d.analysisData.isBackward = pAnalyzeFilter->IsBackward();
	vsapi->freeFrame(evil);
	d.thSAD = d.thSAD / 255.;
	d.nLambda /= 255.;
	int32_t referenceBlockSize = 8 * 8;
	d.thSAD = d.thSAD * (d.analysisData.nBlkSizeX * d.analysisData.nBlkSizeY) / referenceBlockSize;
	if (d.chroma)
		d.thSAD += d.thSAD / (d.analysisData.xRatioUV * d.analysisData.yRatioUV) * 2;
	d.analysisData.nMotionFlags = 0;
	d.analysisData.nMotionFlags |= d.analysisData.isBackward ? MOTION_IS_BACKWARD : 0;
	d.analysisData.nMotionFlags |= d.chroma ? MOTION_USE_CHROMA_MOTION : 0;
	d.analysisData.nPel = d.nSuperPel;//x
	int32_t nSuperWidth = d.supervi->width;
	if (nHeight != d.analysisData.nHeight || nSuperWidth - 2 * d.nSuperHPad != d.analysisData.nWidth) {
		vsapi->setError(out, "Recalculate: wrong frame size.");
		vsapi->freeNode(d.node);
		vsapi->freeNode(d.vectors);
		return d;
	}
	d.analysisData.nHPadding = d.nSuperHPad;
	d.analysisData.nVPadding = d.nSuperVPad;
	int32_t nBlkX = (d.analysisData.nWidth - d.analysisData.nOverlapX) / (d.analysisData.nBlkSizeX - d.analysisData.nOverlapX);//x
	int32_t nBlkY = (d.analysisData.nHeight - d.analysisData.nOverlapY) / (d.analysisData.nBlkSizeY - d.analysisData.nOverlapY);
	d.analysisData.nBlkX = nBlkX;
	d.analysisData.nBlkY = nBlkY;
	d.analysisData.nLvCount = 1;
	if (d.divideExtra) {
		memcpy(&d.analysisDataDivided, &d.analysisData, sizeof(d.analysisData));
		d.analysisDataDivided.nBlkX = d.analysisData.nBlkX * 2;
		d.analysisDataDivided.nBlkY = d.analysisData.nBlkY * 2;
		d.analysisDataDivided.nBlkSizeX = d.analysisData.nBlkSizeX / 2;
		d.analysisDataDivided.nBlkSizeY = d.analysisData.nBlkSizeY / 2;
		d.analysisDataDivided.nOverlapX = d.analysisData.nOverlapX / 2;
		d.analysisDataDivided.nOverlapY = d.analysisData.nOverlapY / 2;
		d.analysisDataDivided.nLvCount = d.analysisData.nLvCount + 1;
	}
	try {
		d.mvClip = new MVClipDicks(d.vectors, 8 * 8 * 255, 255, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Recalculate: ").append(e.what()).c_str());
		vsapi->freeNode(d.node);
		vsapi->freeNode(d.vectors);
		return d;
	}
	return d;
}

static void VS_CC mvrecalculateCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	auto args = ArgumentList{ in };
	auto Core = VaporCore{ core };
	auto super = static_cast<Clip>(args["super"]);
	auto vectors = static_cast<Clip>(args["vectors"]);
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
		auto data = new MVRecalculateData{ CreateVector(argMap, out, vsapi) };
		if (vsapi->getError(out) != nullptr) {
			delete data;
			vsapi->freeMap(argMap);
			vsapi->freeMap(evalMap);
			return Clip{};
		}
		vsapi->createFilter(argMap, evalMap, "Recalculate", mvrecalculateInit, mvrecalculateGetFrame, mvrecalculateFree, fmParallel, 0, data, core);
		auto refined_vec = vsapi->propGetNode(evalMap, "clip", 0, nullptr);
		vsapi->freeMap(argMap);
		vsapi->freeMap(evalMap);
		return Clip{ refined_vec };
	};
	if (super.FrameCount == vectors.FrameCount) {
		auto data = new MVRecalculateData{ CreateVector(in, out, vsapi) };
		if (vsapi->getError(out) != nullptr) {
			delete data;
			return;
		}
		vsapi->createFilter(in, out, "Recalculate", mvrecalculateInit, mvrecalculateGetFrame, mvrecalculateFree, fmParallel, 0, data, core);
	}
	else {
		auto vecCount = vectors.FrameCount / super.FrameCount;
		auto mvmulti = std::vector<Clip>{};
		mvmulti.reserve(vecCount);
		for (auto x : Range{ vecCount }) {
			auto v = Eval(Core["std"]["SelectEvery"]("clip", vectors, "cycle", vecCount, "offsets", x));
			if (v.ContainsVideoReference() == false)
				return;
			mvmulti.push_back(std::move(v));
		}
		auto vecs = Core["std"]["Interleave"]("clips", mvmulti);
		VaporGlobals::API->propSetNode(out, "clip", vecs.VideoNode, VSPropAppendMode::paAppend);
	}
}

void mvrecalculateRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Recalculate",
		"super:clip;"
		"vectors:clip;"
		"thsad:float:opt;"
		"smooth:int:opt;"
		"blksize:int:opt;"
		"blksizev:int:opt;"
		"search:int:opt;"
		"searchparam:int:opt;"
		"lambda:float:opt;"
		"chroma:int:opt;"
		"truemotion:int:opt;"
		"pnew:int:opt;"
		"overlap:int:opt;"
		"overlapv:int:opt;"
		"divide:int:opt;"
		"meander:int:opt;"
		"fields:int:opt;"
		"tff:int:opt;"
		"dct:int:opt;"
		, mvrecalculateCreate, 0, plugin);
}