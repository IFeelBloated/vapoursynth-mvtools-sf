#pragma once
#include <array>
#include <memory>
#include <cmath>
#include "Include/VapourSynth.h"
#include "Include/VSHelper.h"
#include "MaskFun.hpp"
#include "MVFilter.hpp"
#include "SimpleResize.hpp"
#include "Include/Interface.hxx"

struct MVMaskData final {
	self(vsapi, static_cast<const VSAPI *>(nullptr));
	self(node, static_cast<VSNodeRef *>(nullptr));
	self(vi, static_cast<const VSVideoInfo *>(nullptr));
	self(vectors, static_cast<VSNodeRef *>(nullptr));
	self(bleh, static_cast<MVFilter *>(nullptr));
	self(mvClip, static_cast<MVClipDicks *>(nullptr));
	self(upsizer, static_cast<SimpleResize<float> *>(nullptr));
	self(upsizerUV, static_cast<SimpleResize<float> *>(nullptr));
	self(IsGray, false);
	self(Illformed, false);
	self(ml, 0.);
	self(fGamma, 0.);
	self(kind, 0_i64);
	self(time256, 0);
	self(nSceneChangeValue, 0.f);
	self(thscd1, 0.);
	self(thscd2, 0.);
	self(fMaskNormFactor, 0.);
	self(fMaskNormFactor2, 0.);
	self(fHalfGamma, 0.);
	self(nWidthUV, 0);
	self(nHeightUV, 0);
	self(nWidthB, 0);
	self(nHeightB, 0);
	self(nWidthBUV, 0);
	self(nHeightBUV, 0);
	MVMaskData(const VSMap *in, VSMap *out, const VSAPI *api) {
		vsapi = api;
		node = vsapi->propGetNode(in, "clip", 0, nullptr);
		vi = vsapi->getVideoInfo(node);
		vectors = vsapi->propGetNode(in, "vectors", 0, nullptr);
		if (!isConstantFormat(vi) || vi->format->bitsPerSample < 32 || vi->format->sampleType != stFloat) {
			vsapi->setError(out, "Mask: input clip must be single precision, with constant dimensions.");
			Illformed = true;
			return;
		}
		IsGray = vi->format->numPlanes == 1;
		auto err = 0;
		ml = vsapi->propGetFloat(in, "ml", 0, &err);
		if (err)
			ml = 100.;
		fGamma = vsapi->propGetFloat(in, "gamma", 0, &err);
		if (err)
			fGamma = 1.;
		kind = vsapi->propGetInt(in, "kind", 0, &err);
		if (err)
			kind = 0;
		auto time = vsapi->propGetFloat(in, "time", 0, &err);
		if (err)
			time = 100.;
		if (time < 0. || time > 100.) {
			vsapi->setError(out, "Mask: time must be between 0.0 and 100.0 (inclusive).");
			Illformed = true;
			return;
		}
		time256 = static_cast<decltype(time256)>(time * 256. / 100.);
		nSceneChangeValue = static_cast<decltype(nSceneChangeValue)>(vsapi->propGetFloat(in, "ysc", 0, &err));
		if (err)
			nSceneChangeValue = 0.f;
		thscd1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
		if (err)
			thscd1 = MV_DEFAULT_SCD1;
		thscd2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
		if (err)
			thscd2 = MV_DEFAULT_SCD2;
		if (fGamma < 0.) {
			vsapi->setError(out, "Mask: gamma must not be negative.");
			Illformed = true;
			return;
		}
		if (kind < 0 || kind > 5) {
			vsapi->setError(out, "Mask: kind must 0, 1, 2, 3, 4, or 5.");
			Illformed = true;
			return;
		}
		try {
			mvClip = new MVClipDicks{ vectors, thscd1, thscd2, vsapi };
		}
		catch (MVException &e) {
			vsapi->setError(out, e.what());
			Illformed = true;
			return;
		}
		try {
			bleh = new MVFilter{ vectors, "Mask", vsapi };
		}
		catch (MVException &e) {
			vsapi->setError(out, e.what());
			Illformed = true;
			return;
		}
		fMaskNormFactor = 1. / ml;
		fMaskNormFactor2 = fMaskNormFactor * fMaskNormFactor;
		fHalfGamma = fGamma * 0.5;
		nWidthB = bleh->nBlkX * (bleh->nBlkSizeX - bleh->nOverlapX) + bleh->nOverlapX;
		nHeightB = bleh->nBlkY * (bleh->nBlkSizeY - bleh->nOverlapY) + bleh->nOverlapY;
		nHeightUV = bleh->nHeight / bleh->yRatioUV;
		nWidthUV = bleh->nWidth / bleh->xRatioUV;
		nHeightBUV = nHeightB / bleh->yRatioUV;
		nWidthBUV = nWidthB / bleh->xRatioUV;
		upsizer = new SimpleResize<float>(nWidthB, nHeightB, bleh->nBlkX, bleh->nBlkY);
		upsizerUV = new SimpleResize<float>(nWidthBUV, nHeightBUV, bleh->nBlkX, bleh->nBlkY);
	}
	MVMaskData(const MVMaskData &) = delete;
	MVMaskData(MVMaskData &&) = delete;
	auto &operator=(const MVMaskData &) = delete;
	auto &operator=(MVMaskData &&) = delete;
	~MVMaskData() {
		vsapi->freeNode(node);
		vsapi->freeNode(vectors);
		delete mvClip;
		delete bleh;
		delete upsizer;
		delete upsizerUV;
	}
};

static auto VS_CC mvmaskInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	auto d = reinterpret_cast<MVMaskData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

static auto VS_CC mvmaskGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	auto d = reinterpret_cast<MVMaskData *>(*instanceData);
	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->vectors, frameCtx);
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		auto src = vsapi->getFrameFilter(n, d->node, frameCtx);
		auto dst = vsapi->newVideoFrame(d->vi->format, d->vi->width, d->vi->height, src, core);
		auto pSrc = reinterpret_cast<const float *>(vsapi->getReadPtr(src, 0));
		auto nSrcPitch = vsapi->getStride(src, 0) / sizeof(float);
		auto pDst = std::array<float *, 3>{ nullptr, nullptr, nullptr };
		auto nDstPitches = std::array<int, 3>{ 0, 0, 0 };
		for (auto i = 0; i < d->vi->format->numPlanes; ++i) {
			pDst[i] = reinterpret_cast<float *>(vsapi->getWritePtr(dst, i));
			nDstPitches[i] = vsapi->getStride(dst, i) / sizeof(float);
		}
		auto mvn = vsapi->getFrameFilter(n, d->vectors, frameCtx);
		auto balls = MVClipBalls{ d->mvClip, vsapi };
		balls.Update(mvn);
		vsapi->freeFrame(mvn);
		auto kind = d->kind;
		auto nWidth = d->bleh->nWidth;
		auto nHeight = d->bleh->nHeight;
		auto nWidthUV = d->nWidthUV;
		auto nHeightUV = d->nHeightUV;
		auto nSceneChangeValue = d->nSceneChangeValue;
		auto nBlkX = d->bleh->nBlkX;
		auto nBlkY = d->bleh->nBlkY;
		auto nBlkCount = d->bleh->nBlkCount;
		auto fMaskNormFactor = d->fMaskNormFactor;
		auto fMaskNormFactor2 = d->fMaskNormFactor2;
		auto fGamma = d->fGamma;
		auto fHalfGamma = d->fHalfGamma;
		auto nPel = d->bleh->nPel;
		auto nBlkSizeX = d->bleh->nBlkSizeX;
		auto nBlkSizeY = d->bleh->nBlkSizeY;
		auto nOverlapX = d->bleh->nOverlapX;
		auto nOverlapY = d->bleh->nOverlapY;
		auto nWidthB = d->nWidthB;
		auto nHeightB = d->nHeightB;
		auto nWidthBUV = d->nWidthBUV;
		auto nHeightBUV = d->nHeightBUV;
		auto time256 = d->time256;
		auto upsizer = d->upsizer;
		auto upsizerUV = d->upsizerUV;
		auto smallMask = std::unique_ptr<float[]>(new float[nBlkX * nBlkY]);
		auto smallMaskV = std::unique_ptr<float[]>(new float[nBlkX * nBlkY]);
		auto mask = std::unique_ptr<double[]>(new double[nBlkX * nBlkY]);
		auto VectorToSmallMask = [&]() {
			auto mvmaskLength = [&](auto &&v) {
				auto pel = d->mvClip->GetPel();
				auto norme = (static_cast<double>(v.x) * v.x + static_cast<double>(v.y) * v.y) / (pel * pel);
				return std::pow(norme * fMaskNormFactor2, fHalfGamma);
			};
			switch (kind) {
			case 0:
				for (auto i = 0; i < nBlkCount; ++i)
					smallMask[i] = static_cast<decltype(smallMask[0] + 0)>(mvmaskLength(balls[0][i].GetMV()));
				break;
			case 1:
				MakeSADMaskTime(balls, nBlkX, nBlkY, 4.0 * fMaskNormFactor / (nBlkSizeX * nBlkSizeY), fGamma, nPel, mask.get(), nBlkX, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
				for (auto i = 0; i < nBlkX * nBlkY; ++i)
					smallMask[i] = static_cast<decltype(smallMask[0] + 0)>(mask[i] / 255.);
				break;
			case 2:
				MakeVectorOcclusionMaskTime(balls, d->mvClip->IsBackward(), nBlkX, nBlkY, 1.0 / fMaskNormFactor, fGamma, nPel, mask.get(), nBlkX, time256, nBlkSizeX - nOverlapX, nBlkSizeY - nOverlapY);
				for (auto i = 0; i < nBlkX * nBlkY; ++i)
					smallMask[i] = static_cast<decltype(smallMask[0] + 0)>(mask[i] / 255.);
				break;
			case 3:
				for (auto i = 0; i < nBlkCount; ++i)
					smallMask[i] = static_cast<decltype(smallMask[0] + 0)>((balls[0][i].GetMV().x * fMaskNormFactor * 100. + 128.) / 255.);
				break;
			case 4:
				for (auto i = 0; i < nBlkCount; ++i)
					smallMask[i] = static_cast<decltype(smallMask[0] + 0)>((balls[0][i].GetMV().y * fMaskNormFactor * 100. + 128.) / 255.);
				break;
			case 5:
				for (auto i = 0; i < nBlkCount; ++i) {
					auto &&v = balls[0][i].GetMV();
					smallMask[i] = static_cast<decltype(smallMask[0] + 0)>((v.x * fMaskNormFactor * 100. + 128.) / 255.);
					smallMaskV[i] = static_cast<decltype(smallMask[0] + 0)>((v.y * fMaskNormFactor * 100. + 128.) / 255.);
				}
				break;
			default:
				break;
			}
		};
		auto SmallMaskToLuma = [&]() {
			if (kind == 5)
				std::memcpy(pDst[0], pSrc, nSrcPitch * nHeight * sizeof(float));
			else {
				upsizer->Resize(pDst[0], nDstPitches[0], smallMask.get(), nBlkX);
				if (nWidth > nWidthB)
					for (auto h = 0; h < nHeight; ++h)
						for (auto w = nWidthB; w < nWidth; ++w)
							pDst[0][h * nDstPitches[0] + w] = pDst[0][h * nDstPitches[0] + nWidthB - 1];
				if (nHeight > nHeightB)
					vs_bitblt(pDst[0] + nHeightB * nDstPitches[0], nDstPitches[0] * sizeof(float), pDst[0] + (nHeightB - 1) * nDstPitches[0], nDstPitches[0] * sizeof(float), nWidth * sizeof(float), nHeight - nHeightB);
			}
		};
		auto SmallMaskToChroma = [&]() {
			if (!d->IsGray) {
				upsizerUV->Resize(pDst[1], nDstPitches[1], smallMask.get(), nBlkX);
				if (kind == 5)
					upsizerUV->Resize(pDst[2], nDstPitches[2], smallMaskV.get(), nBlkX);
				else
					std::memcpy(pDst[2], pDst[1], nHeightUV * nDstPitches[1] * sizeof(float));
				if (nWidthUV > nWidthBUV)
					for (auto h = 0; h < nHeightUV; ++h)
						for (auto w = nWidthBUV; w < nWidthUV; ++w) {
							pDst[1][h * nDstPitches[1] + w] = pDst[1][h * nDstPitches[1] + nWidthBUV - 1];
							pDst[2][h * nDstPitches[2] + w] = pDst[2][h * nDstPitches[2] + nWidthBUV - 1];
						}
				if (nHeightUV > nHeightBUV) {
					vs_bitblt(pDst[1] + nHeightBUV * nDstPitches[1], nDstPitches[1] * sizeof(float), pDst[1] + (nHeightBUV - 1) * nDstPitches[1], nDstPitches[1] * sizeof(float), nWidthUV * sizeof(float), nHeightUV - nHeightBUV);
					vs_bitblt(pDst[2] + nHeightBUV * nDstPitches[2], nDstPitches[2] * sizeof(float), pDst[2] + (nHeightBUV - 1) * nDstPitches[2], nDstPitches[2] * sizeof(float), nWidthUV * sizeof(float), nHeightUV - nHeightBUV);
				}
			}
			else
				return;
		};
		auto SceneChangeValueToLuma = [&]() {
			if (kind == 5)
				std::memcpy(pDst[0], pSrc, nSrcPitch * nHeight * sizeof(float));
			else
				for (auto i = 0; i < nHeight * nDstPitches[0]; ++i)
					pDst[0][i] = nSceneChangeValue;
		};
		auto SceneChangeValueToChroma = [&]() {
			if (!d->IsGray) {
				for (auto i = 0; i < nHeightUV * nDstPitches[1]; ++i)
					pDst[1][i] = nSceneChangeValue;
				for (auto i = 0; i < nHeightUV * nDstPitches[2]; ++i)
					pDst[2][i] = nSceneChangeValue;
			}
			else
				return;
		};
		if (balls.IsUsable()) {
			VectorToSmallMask();
			SmallMaskToLuma();
			SmallMaskToChroma();
		}
		else {
			SceneChangeValueToLuma();
			SceneChangeValueToChroma();
		}
		vsapi->freeFrame(src);
		return PointerAddConstant(dst);
	}
	return static_cast<const VSFrameRef *>(nullptr);
}

static auto VS_CC mvmaskFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	auto d = reinterpret_cast<MVMaskData *>(instanceData);
	delete d;
}

static auto VS_CC mvmaskCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	auto data = new MVMaskData{ in, out, vsapi };
	if (!data->Illformed)
		vsapi->createFilter(in, out, "Mask", mvmaskInit, mvmaskGetFrame, mvmaskFree, fmParallel, 0, data, core);
	else
		delete data;
}

auto mvmaskRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Mask",
		"clip:clip;"
		"vectors:clip;"
		"ml:float:opt;"
		"gamma:float:opt;"
		"kind:int:opt;"
		"time:float:opt;"
		"ysc:float:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvmaskCreate, 0, plugin);
}