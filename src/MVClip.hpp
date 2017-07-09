#pragma once
#include <string>
#include <utility>
#include "FakeGroupOfPlanes.hpp"
#include "Cosmetics.hpp"

class MVClipDicks final :public MVAnalysisData {
	self(nBlkCount, 0_i32);
	self(nSCD1, 0.);
	self(nSCD2, 0.);
public:
	MVClipDicks() = default;
	MVClipDicks(VSNodeRef *vectors, double _nSCD1, double _nSCD2, const VSAPI *vsapi) {
		using namespace std::literals;
		constexpr auto MaxErrorLength = 1024;
		constexpr auto maxSAD = 256. * 256. * 255.;
		constexpr auto referenceBlockSize = 8 * 8;
		constexpr auto HeaderOffset = sizeof(std::int32_t);
		auto errorMsg = ""s;
		errorMsg.reserve(MaxErrorLength);
		auto evil = vsapi->getFrame(0, vectors, errorMsg.data(), MaxErrorLength);
		if (evil == nullptr)
			throw MVException{ "Failed to retrieve first frame from some motion clip. Error message: " + errorMsg };
		auto pAnalyzeFilter = reinterpret_cast<const MVAnalysisData *>(vsapi->getReadPtr(evil, 0) + HeaderOffset);
		if (pAnalyzeFilter->GetMagicKey() != MotionMagicKey) {
			vsapi->freeFrame(evil);
			throw MVException{ "Invalid motion vector clip." };
		}
		if (_nSCD1 > maxSAD)
			throw MVException{ "thscd1 can be at most " + std::to_string(maxSAD) + "." };
		nBlkSizeX = pAnalyzeFilter->GetBlkSizeX();
		nBlkSizeY = pAnalyzeFilter->GetBlkSizeY();
		nPel = pAnalyzeFilter->GetPel();
		isBackward = pAnalyzeFilter->IsBackward();
		nLvCount = pAnalyzeFilter->GetLevelCount();
		nDeltaFrame = pAnalyzeFilter->GetDeltaFrame();
		nWidth = pAnalyzeFilter->GetWidth();
		nHeight = pAnalyzeFilter->GetHeight();
		nMagicKey = pAnalyzeFilter->GetMagicKey();
		nOverlapX = pAnalyzeFilter->GetOverlapX();
		nOverlapY = pAnalyzeFilter->GetOverlapY();
		xRatioUV = pAnalyzeFilter->GetXRatioUV();
		yRatioUV = pAnalyzeFilter->GetYRatioUV();
		nVPadding = pAnalyzeFilter->GetVPadding();
		nHPadding = pAnalyzeFilter->GetHPadding();
		nMotionFlags = pAnalyzeFilter->GetMotionFlags();
		nBlkX = pAnalyzeFilter->GetBlkX();
		nBlkY = pAnalyzeFilter->GetBlkY();
		nBlkCount = nBlkX * nBlkY;
		nSCD1 = _nSCD1 * (nBlkSizeX * nBlkSizeY) / referenceBlockSize;
		if (pAnalyzeFilter->IsChromaMotion())
			nSCD1 += nSCD1 / (xRatioUV * yRatioUV) * 2;
		nSCD1 = nSCD1 / 255.;
		nSCD2 = _nSCD2 * nBlkCount / 256.;
		vsapi->freeFrame(evil);
	}
	MVClipDicks(MVClipDicks &&) = default;
	MVClipDicks(const MVClipDicks &) = default;
	auto operator=(MVClipDicks &&)->decltype(*this) = default;
	auto operator=(const MVClipDicks &)->decltype(*this) = default;
	~MVClipDicks() = default;
	auto GetBlkCount() const {
		return nBlkCount;
	}
	auto GetThSCD1() const {
		return nSCD1;
	}
	auto GetThSCD2() const {
		return nSCD2;
	}
};

class MVClipBalls final :public FakeGroupOfPlanes {
	self(dicks, static_cast<MVClipDicks *>(nullptr));
	self(vsapi, static_cast<const VSAPI *>(nullptr));
public:
	MVClipBalls() = default;
	MVClipBalls(MVClipDicks *_dicks, const VSAPI *_vsapi) :FakeGroupOfPlanes{ _dicks->GetBlkSizeX(), _dicks->GetBlkSizeY(), _dicks->GetLevelCount(), _dicks->GetPel(), _dicks->GetOverlapX(), _dicks->GetOverlapY(), _dicks->GetYRatioUV(), _dicks->GetBlkX(), _dicks->GetBlkY() } {
		dicks = _dicks;
		vsapi = _vsapi;
	}
	auto &operator=(MVClipBalls &&OtherMVClipBalls) {
		if (this != &OtherMVClipBalls) {
			dicks = OtherMVClipBalls.dicks;
			vsapi = OtherMVClipBalls.vsapi;
			static_cast<FakeGroupOfPlanes &>(*this) = static_cast<FakeGroupOfPlanes &&>(OtherMVClipBalls);
		}
		return *this;
	}
	auto &operator=(const MVClipBalls &) = delete;
	MVClipBalls(MVClipBalls &&OtherMVClipBalls) {
		*this = std::move(OtherMVClipBalls);
	}
	MVClipBalls(const MVClipBalls &) = delete;
	~MVClipBalls() = default;
	auto Update(const VSFrameRef *fn) {
		auto pMv = reinterpret_cast<const std::int32_t *>(vsapi->getReadPtr(fn, 0));
		auto _headerSize = pMv[0] / sizeof(std::int32_t);
		auto nMagicKey = pMv[1];
		auto nVersion = pMv[2];
		if (nMagicKey != MotionMagicKey)
			throw MVException{ "MVTools: invalid motion vector clip. Who knows where this error came from exactly?" };
		if (nVersion != MVAnalysisDataVersion)
			throw MVException{ "MVTools: incompatible version of motion vector clip. Who knows where this error came from exactly?" };
		UpdateAllLevels(pMv + _headerSize);
	}
	auto IsUsable() const {
		auto NotSceneChange = !IsSceneChange(dicks->GetThSCD1(), dicks->GetThSCD2());
		return NotSceneChange && IsValid();
	}
};