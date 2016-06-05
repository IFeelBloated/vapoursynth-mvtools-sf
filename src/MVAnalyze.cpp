#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include "VapourSynth.h"
#include "VSHelper.h"
#include "DCTFFTW.h"
#include "GroupOfPlanes.h"
#include "MVInterface.h"

struct MVAnalyzeData {
	VSNodeRef *node;
	VSVideoInfo vi;
	const VSVideoInfo *supervi;
	MVAnalysisData analysisData;
	MVAnalysisData analysisDataDivided;
	double nLambda;
	SearchType searchType;
	SearchType searchTypeCoarse;
	int32_t nSearchParam;
	int32_t nPelSearch;
	double lsad;
	int32_t pnew;
	int32_t plen;
	int32_t plevel;
	bool global;
	int32_t pglobal;
	int32_t pzero;
	int32_t divideExtra;
	double badSAD;
	int32_t badrange;
	bool meander;
	bool tryMany;
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
	int32_t levels;
	int32_t search;
	int32_t search_coarse;
	int32_t searchparam;
	bool isb;
	bool chroma;
	int32_t delta;
	bool truemotion;
	int32_t overlap;
	int32_t overlapv;
	bool fields;
	bool tff;
	int32_t tffexists;
};

static void VS_CC mvanalyzeInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVAnalyzeData *d = reinterpret_cast<MVAnalyzeData *>(*instanceData);
	vsapi->setVideoInfo(&d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvanalyzeGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVAnalyzeData *d = reinterpret_cast<MVAnalyzeData *>(*instanceData);
	if (activationReason == arInitial) {
		int32_t nref;
		if (d->analysisData.nDeltaFrame > 0) {
			int32_t offset = (d->analysisData.isBackward) ? d->analysisData.nDeltaFrame : -d->analysisData.nDeltaFrame;
			nref = n + offset;
			if (nref >= 0 && (nref < d->vi.numFrames || !d->vi.numFrames)) {
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
		else {
			nref = -d->analysisData.nDeltaFrame;
			if (n < nref) {
				vsapi->requestFrameFilter(n, d->node, frameCtx);
				vsapi->requestFrameFilter(nref, d->node, frameCtx);
			}
			else {
				vsapi->requestFrameFilter(nref, d->node, frameCtx);
				vsapi->requestFrameFilter(n, d->node, frameCtx);
			}
		}
	}
	else if (activationReason == arAllFramesReady) {
		GroupOfPlanes *vectorFields = new GroupOfPlanes(d->analysisData.nBlkSizeX, d->analysisData.nBlkSizeY, d->analysisData.nLvCount, d->analysisData.nPel, d->analysisData.nMotionFlags, d->analysisData.nOverlapX, d->analysisData.nOverlapY, d->analysisData.nBlkX, d->analysisData.nBlkY, d->analysisData.xRatioUV, d->analysisData.yRatioUV, d->divideExtra);
		const uint8_t *pSrc[3] = { nullptr };
		const uint8_t *pRef[3] = { nullptr };
		uint8_t *pDst = { nullptr };
		int32_t nSrcPitch[3] = { 0 };
		int32_t nRefPitch[3] = { 0 };
		int32_t nref;
		if (d->analysisData.nDeltaFrame > 0) {
			int32_t offset = (d->analysisData.isBackward) ? d->analysisData.nDeltaFrame : -d->analysisData.nDeltaFrame;
			nref = n + offset;
		}
		else
			nref = -d->analysisData.nDeltaFrame;
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		const VSMap *srcprops = vsapi->getFramePropsRO(src);
		int err;
		bool srctff = !!vsapi->propGetInt(srcprops, "_Field", 0, &err);
		if (err && d->fields && !d->tffexists) {
			vsapi->setFilterError("Analyze: _Field property not found in input frame. Therefore, you must pass tff argument.", frameCtx);
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
		VSFrameRef *dst = vsapi->newVideoFrame(d->vi.format, dst_width, dst_height, src, core);
		pDst = vsapi->getWritePtr(dst, 0);
		memcpy(pDst, &d->headerSize, sizeof(int32_t));
		if (d->divideExtra)
			memcpy(pDst + sizeof(int32_t), &d->analysisDataDivided, sizeof(d->analysisData));
		else
			memcpy(pDst + sizeof(int32_t), &d->analysisData, sizeof(d->analysisData));
		pDst += d->headerSize;
		if (nref >= 0 && (nref < d->vi.numFrames || !d->vi.numFrames)) {
			const VSFrameRef *ref = vsapi->getFrameFilter(nref, d->node, frameCtx);
			const VSMap *refprops = vsapi->getFramePropsRO(ref);
			bool reftff = !!vsapi->propGetInt(refprops, "_Field", 0, &err);
			if (err && d->fields && !d->tffexists) {
				vsapi->setFilterError("Analyze: _Field property not found in input frame. Therefore, you must pass tff argument.", frameCtx);
				delete vectorFields;
				vsapi->freeFrame(src);
				vsapi->freeFrame(ref);
				vsapi->freeFrame(dst);
				return nullptr;
			}
			if (d->tffexists)
				reftff = !!(static_cast<int>(d->tff) ^ (nref % 2));
			int32_t fieldShift = 0;
			if (d->fields && d->analysisData.nPel > 1 && (d->analysisData.nDeltaFrame % 2))
				fieldShift = (srctff && !reftff) ? d->analysisData.nPel / 2 : ((reftff && !srctff) ? -(d->analysisData.nPel / 2) : 0);
			for (int32_t plane = 0; plane < d->supervi->format->numPlanes; plane++) {
				pRef[plane] = vsapi->getReadPtr(ref, plane);
				nRefPitch[plane] = vsapi->getStride(ref, plane);
			}
			MVGroupOfFrames *pSrcGOF = new MVGroupOfFrames(d->nSuperLevels, d->analysisData.nWidth, d->analysisData.nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, d->analysisData.xRatioUV, d->analysisData.yRatioUV);
			MVGroupOfFrames *pRefGOF = new MVGroupOfFrames(d->nSuperLevels, d->analysisData.nWidth, d->analysisData.nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, d->analysisData.xRatioUV, d->analysisData.yRatioUV);
			pSrcGOF->Update(d->nModeYUV, (uint8_t *)pSrc[0], nSrcPitch[0], (uint8_t *)pSrc[1], nSrcPitch[1], (uint8_t *)pSrc[2], nSrcPitch[2]);
			pRefGOF->Update(d->nModeYUV, (uint8_t *)pRef[0], nRefPitch[0], (uint8_t *)pRef[1], nRefPitch[1], (uint8_t *)pRef[2], nRefPitch[2]);
			DCTClass *DCTc = nullptr;
			if (d->dctmode != 0)
				DCTc = new DCTFFTW(d->blksize, d->blksizev, d->dctmode);
			vectorFields->SearchMVs(pSrcGOF, pRefGOF, d->searchType, d->nSearchParam, d->nPelSearch, d->nLambda, d->lsad, d->pnew, d->plevel, d->global, reinterpret_cast<int32_t*>(pDst), nullptr, fieldShift, DCTc, d->pzero, d->pglobal, d->badSAD, d->badrange, d->meander, nullptr, d->tryMany, d->searchTypeCoarse);
			if (d->divideExtra)
				vectorFields->ExtraDivide(reinterpret_cast<int32_t*>(pDst));
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

static void VS_CC mvanalyzeFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVAnalyzeData *d = reinterpret_cast<MVAnalyzeData *>(instanceData);
	vsapi->freeNode(d->node);
	delete d;
}

static void VS_CC mvanalyzeCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	MVAnalyzeData d;
	MVAnalyzeData *data;
	int err;
	d.blksize = int64ToIntS(vsapi->propGetInt(in, "blksize", 0, &err));
	if (err)
		d.blksize = 8;
	d.blksizev = int64ToIntS(vsapi->propGetInt(in, "blksizev", 0, &err));
	if (err)
		d.blksizev = d.blksize;
	d.levels = int64ToIntS(vsapi->propGetInt(in, "levels", 0, &err));
	d.search = int64ToIntS(vsapi->propGetInt(in, "search", 0, &err));
	if (err)
		d.search = 4;
	d.search_coarse = int64ToIntS(vsapi->propGetInt(in, "search_coarse", 0, &err));
	if (err)
		d.search_coarse = 3;
	d.searchparam = int64ToIntS(vsapi->propGetInt(in, "searchparam", 0, &err));
	if (err)
		d.searchparam = 2;
	d.nPelSearch = int64ToIntS(vsapi->propGetInt(in, "pelsearch", 0, &err));
	d.isb = !!vsapi->propGetInt(in, "isb", 0, &err);
	d.chroma = !!vsapi->propGetInt(in, "chroma", 0, &err);
	if (err)
		d.chroma = 1;
	d.delta = int64ToIntS(vsapi->propGetInt(in, "delta", 0, &err));
	if (err)
		d.delta = 1;
	d.truemotion = !!vsapi->propGetInt(in, "truemotion", 0, &err);
	if (err)
		d.truemotion = 1;
	d.nLambda = vsapi->propGetFloat(in, "lambda", 0, &err);
	if (err)
		d.nLambda = d.truemotion ? (1000 * d.blksize * d.blksizev / 64) : 0.;
	d.lsad = vsapi->propGetFloat(in, "lsad", 0, &err);
	if (err)
		d.lsad = d.truemotion ? 1200. : 400.;
	d.plevel = int64ToIntS(vsapi->propGetInt(in, "plevel", 0, &err));
	if (err)
		d.plevel = d.truemotion ? 1 : 0;
	d.global = !!vsapi->propGetInt(in, "global", 0, &err);
	if (err)
		d.global = d.truemotion ? 1 : 0;
	d.pnew = int64ToIntS(vsapi->propGetInt(in, "pnew", 0, &err));
	if (err)
		d.pnew = d.truemotion ? 50 : 0;
	d.pzero = int64ToIntS(vsapi->propGetInt(in, "pzero", 0, &err));
	if (err)
		d.pzero = d.pnew;
	d.pglobal = int64ToIntS(vsapi->propGetInt(in, "pglobal", 0, &err));
	d.overlap = int64ToIntS(vsapi->propGetInt(in, "overlap", 0, &err));
	d.overlapv = int64ToIntS(vsapi->propGetInt(in, "overlapv", 0, &err));
	if (err)
		d.overlapv = d.overlap;
	d.dctmode = int64ToIntS(vsapi->propGetInt(in, "dct", 0, &err));
	d.divideExtra = int64ToIntS(vsapi->propGetInt(in, "divide", 0, &err));
	d.badSAD = vsapi->propGetFloat(in, "badsad", 0, &err);
	if (err)
		d.badSAD = 10000.;
	d.badrange = int64ToIntS(vsapi->propGetInt(in, "badrange", 0, &err));
	if (err)
		d.badrange = 24;
	d.meander = !!vsapi->propGetInt(in, "meander", 0, &err);
	if (err)
		d.meander = 1;
	d.tryMany = !!vsapi->propGetInt(in, "trymany", 0, &err);
	d.fields = !!vsapi->propGetInt(in, "fields", 0, &err);
	d.tff = !!vsapi->propGetInt(in, "tff", 0, &err);
	d.tffexists = !err;
	if (d.search < 0 || d.search > 7) {
		vsapi->setError(out, "Analyze: search must be between 0 and 7 (inclusive).");
		return;
	}
	if (d.search_coarse < 0 || d.search_coarse > 7) {
		vsapi->setError(out, "Analyze: search_coarse must be between 0 and 7 (inclusive).");
		return;
	}
	if (d.dctmode < 0 || d.dctmode > 10) {
		vsapi->setError(out, "Analyze: dct must be between 0 and 10 (inclusive).");
		return;
	}
	if (d.dctmode >= 5 && !((d.blksize == 4 && d.blksizev == 4) ||
		(d.blksize == 8 && d.blksizev == 8) ||
		(d.blksize == 16 && d.blksizev == 16) ||
		(d.blksize == 32 && d.blksizev == 32))) {
		vsapi->setError(out, "Analyze: dct 5..10 can only work with 4x4, 8x8, 16x16, and 32x32 blocks.");
		return;
	}
	if (d.divideExtra < 0 || d.divideExtra > 2) {
		vsapi->setError(out, "Analyze: divide must be between 0 and 2 (inclusive).");
		return;
	}
	d.analysisData.nBlkSizeX = d.blksize;
	d.analysisData.nBlkSizeY = d.blksizev;
	if ((d.analysisData.nBlkSizeX != 4 || d.analysisData.nBlkSizeY != 4) &&
		(d.analysisData.nBlkSizeX != 8 || d.analysisData.nBlkSizeY != 4) &&
		(d.analysisData.nBlkSizeX != 8 || d.analysisData.nBlkSizeY != 8) &&
		(d.analysisData.nBlkSizeX != 16 || d.analysisData.nBlkSizeY != 2) &&
		(d.analysisData.nBlkSizeX != 16 || d.analysisData.nBlkSizeY != 8) &&
		(d.analysisData.nBlkSizeX != 16 || d.analysisData.nBlkSizeY != 16) &&
		(d.analysisData.nBlkSizeX != 32 || d.analysisData.nBlkSizeY != 32) &&
		(d.analysisData.nBlkSizeX != 32 || d.analysisData.nBlkSizeY != 16)) {
		vsapi->setError(out, "Analyze: the block size must be 4x4, 8x4, 8x8, 16x2, 16x8, 16x16, 32x16, or 32x32.");
		return;
	}
	d.analysisData.nDeltaFrame = d.delta;
	if (d.plevel < 0 || d.plevel > 2) {
		vsapi->setError(out, "Analyze: plevel must be between 0 and 2 (inclusive).");
		return;
	}
	if (d.pnew < 0 || d.pnew > 256) {
		vsapi->setError(out, "Analyze: pnew must be between 0 and 256 (inclusive).");
		return;
	}
	if (d.pzero < 0 || d.pzero > 256) {
		vsapi->setError(out, "Analyze: pzero must be between 0 and 256 (inclusive).");
		return;
	}
	if (d.pglobal < 0 || d.pglobal > 256) {
		vsapi->setError(out, "Analyze: pglobal must be between 0 and 256 (inclusive).");
		return;
	}
	if (d.overlap < 0 || d.overlap > d.blksize / 2 ||
		d.overlapv < 0 || d.overlapv > d.blksizev / 2) {
		vsapi->setError(out, "Analyze: overlap must be at most half of blksize, overlapv must be at most half of blksizev, and they both need to be at least 0.");
		return;
	}
	if (d.divideExtra && (d.blksize < 8 || d.blksizev < 8)) {
		vsapi->setError(out, "Analyze: blksize and blksizev must be at least 8 when divide=True.");
		return;
	}
	d.analysisData.nOverlapX = d.overlap;
	d.analysisData.nOverlapY = d.overlapv;
	d.analysisData.isBackward = d.isb;
	SearchType searchTypes[] = { ONETIME, NSTEP, LOGARITHMIC, EXHAUSTIVE, HEX2SEARCH, UMHSEARCH, HSEARCH, VSEARCH };
	d.searchType = searchTypes[d.search];
	d.searchTypeCoarse = searchTypes[d.search_coarse];
	if (d.searchType == NSTEP)
		d.nSearchParam = (d.searchparam < 0) ? 0 : d.searchparam;
	else
		d.nSearchParam = (d.searchparam < 1) ? 1 : d.searchparam;
	d.analysisData.nMagicKey = MOTION_MAGIC_KEY;
	d.analysisData.nVersion = MVANALYSIS_DATA_VERSION;
	d.headerSize = VSMAX(4 + sizeof(d.analysisData), 256);
	d.node = vsapi->propGetNode(in, "super", 0, 0);
	d.supervi = vsapi->getVideoInfo(d.node);
	d.vi = *d.supervi;
	if (!isConstantFormat(&d.vi) || d.vi.format->bitsPerSample < 32 || d.vi.format->sampleType != stFloat) {
		vsapi->setError(out, "Analyze: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.node);
		return;
	}
	if (d.vi.format->colorFamily == cmGray)
		d.chroma = 0;
	if (d.vi.format->colorFamily == cmRGB)
		d.chroma = 1;
	d.nModeYUV = d.chroma ? YUVPLANES : YPLANE;
	d.lsad = d.lsad / 255.;
	d.badSAD = d.badSAD / 255.;
	d.lsad = d.lsad * (d.blksize * d.blksizev) / 64;
	d.badSAD = d.badSAD * (d.blksize * d.blksizev) / 64;
	d.analysisData.nMotionFlags = 0;
	d.analysisData.nMotionFlags |= d.analysisData.isBackward ? MOTION_IS_BACKWARD : 0;
	d.analysisData.nMotionFlags |= d.chroma ? MOTION_USE_CHROMA_MOTION : 0;
	if (d.overlap % (1 << d.vi.format->subSamplingW) ||
		d.overlapv % (1 << d.vi.format->subSamplingH)) {
		vsapi->setError(out, "Analyze: The requested overlap is incompatible with the super clip's subsampling.");
		vsapi->freeNode(d.node);
		return;
	}
	if (d.divideExtra && (d.overlap % (2 << d.vi.format->subSamplingW) ||
		d.overlapv % (2 << d.vi.format->subSamplingH))) {
		vsapi->setError(out, "Analyze: overlap and overlapv must be multiples of 2 or 4 when divide=True, depending on the super clip's subsampling.");
		vsapi->freeNode(d.node);
		return;
	}
	if (d.analysisData.nDeltaFrame <= 0 && d.vi.numFrames && (-d.analysisData.nDeltaFrame) >= d.vi.numFrames) {
		vsapi->setError(out, "Analyze: delta points to frame past the input clip's end.");
		vsapi->freeNode(d.node);
		return;
	}
	d.analysisData.yRatioUV = 1 << d.vi.format->subSamplingH;
	d.analysisData.xRatioUV = 1 << d.vi.format->subSamplingW;
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.node, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Analyze: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.node);
		return;
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
	for (int32_t i = 0; i < 6; ++i)
		if (evil_err[i]) {
			vsapi->setError(out, "Analyze: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.node);
			return;
		}
	if (nHeight <= 0 || d.nSuperHPad < 0 || d.nSuperHPad >= d.vi.width / 2 ||
		d.nSuperVPad < 0 || d.nSuperPel < 1 || d.nSuperPel > 4 ||
		d.nSuperModeYUV < 0 || d.nSuperModeYUV > YUVPLANES || d.nSuperLevels < 1) {
		vsapi->setError(out, "Analyze: parameters from super clip appear to be wrong.");
		vsapi->freeNode(d.node);
		return;
	}
	if ((d.nModeYUV & d.nSuperModeYUV) != d.nModeYUV) {
		vsapi->setError(out, "Analyze: super clip does not contain needed color data.");
		vsapi->freeNode(d.node);
		return;
	}
	d.analysisData.nWidth = d.vi.width - d.nSuperHPad * 2;
	d.analysisData.nHeight = nHeight;
	d.analysisData.nPel = d.nSuperPel;
	d.analysisData.nHPadding = d.nSuperHPad;
	d.analysisData.nVPadding = d.nSuperVPad;
	int32_t nBlkX = (d.analysisData.nWidth - d.analysisData.nOverlapX) / (d.analysisData.nBlkSizeX - d.analysisData.nOverlapX);
	int32_t nBlkY = (d.analysisData.nHeight - d.analysisData.nOverlapY) / (d.analysisData.nBlkSizeY - d.analysisData.nOverlapY);
	d.analysisData.nBlkX = nBlkX;
	d.analysisData.nBlkY = nBlkY;
	int32_t nWidth_B = (d.analysisData.nBlkSizeX - d.analysisData.nOverlapX) * nBlkX + d.analysisData.nOverlapX;
	int32_t nHeight_B = (d.analysisData.nBlkSizeY - d.analysisData.nOverlapY) * nBlkY + d.analysisData.nOverlapY;
	int32_t nLevelsMax = 0;
	while (((nWidth_B >> nLevelsMax) - d.analysisData.nOverlapX) / (d.analysisData.nBlkSizeX - d.analysisData.nOverlapX) > 0 &&
		((nHeight_B >> nLevelsMax) - d.analysisData.nOverlapY) / (d.analysisData.nBlkSizeY - d.analysisData.nOverlapY) > 0)
		nLevelsMax++;
	d.analysisData.nLvCount = d.levels > 0 ? d.levels : nLevelsMax + d.levels;
	if (d.analysisData.nLvCount < 1 || d.analysisData.nLvCount > nLevelsMax) {
		vsapi->setError(out, "Analyze: invalid number of levels.");
		vsapi->freeNode(d.node);
		return;
	}
	if (d.analysisData.nLvCount > d.nSuperLevels) {
		vsapi->setError(out, ("Analyze: super clip has " + std::to_string(d.nSuperLevels) + " levels. Analyze needs " + std::to_string(d.analysisData.nLvCount) + " levels.").c_str());
		vsapi->freeNode(d.node);
		return;
	}
	if (d.nPelSearch <= 0)
		d.nPelSearch = d.analysisData.nPel;
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
	d.vi.width = d.vi.height = 0;
	d.vi.format = vsapi->getFormatPreset(pfGray8, core);
	data = new MVAnalyzeData;
	*data = d;
	vsapi->createFilter(in, out, "Analyze", mvanalyzeInit, mvanalyzeGetFrame, mvanalyzeFree, fmParallel, 0, data, core);
}

void mvanalyzeRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Analyze",
		"super:clip;"
		"blksize:int:opt;"
		"blksizev:int:opt;"
		"levels:int:opt;"
		"search:int:opt;"
		"searchparam:int:opt;"
		"pelsearch:int:opt;"
		"isb:int:opt;"
		"lambda:float:opt;"
		"chroma:int:opt;"
		"delta:int:opt;"
		"truemotion:int:opt;"
		"lsad:float:opt;"
		"plevel:int:opt;"
		"global:int:opt;"
		"pnew:int:opt;"
		"pzero:int:opt;"
		"pglobal:int:opt;"
		"overlap:int:opt;"
		"overlapv:int:opt;"
		"divide:int:opt;"
		"badsad:float:opt;"
		"badrange:int:opt;"
		"meander:int:opt;"
		"trymany:int:opt;"
		"fields:int:opt;"
		"tff:int:opt;"
		"search_coarse:int:opt;"
		"dct:int:opt;"
		, mvanalyzeCreate, 0, plugin);
}

void mvanalyseRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Analyse",
		"super:clip;"
		"blksize:int:opt;"
		"blksizev:int:opt;"
		"levels:int:opt;"
		"search:int:opt;"
		"searchparam:int:opt;"
		"pelsearch:int:opt;"
		"isb:int:opt;"
		"lambda:float:opt;"
		"chroma:int:opt;"
		"delta:int:opt;"
		"truemotion:int:opt;"
		"lsad:float:opt;"
		"plevel:int:opt;"
		"global:int:opt;"
		"pnew:int:opt;"
		"pzero:int:opt;"
		"pglobal:int:opt;"
		"overlap:int:opt;"
		"overlapv:int:opt;"
		"divide:int:opt;"
		"badsad:float:opt;"
		"badrange:int:opt;"
		"meander:int:opt;"
		"trymany:int:opt;"
		"fields:int:opt;"
		"tff:int:opt;"
		"search_coarse:int:opt;"
		"dct:int:opt;"
		, mvanalyzeCreate, 0, plugin);
}