#pragma once
#include <utility>
#include <type_traits>
#include "FakePlaneOfBlocks.hpp"
#include "Cosmetics.hpp"

class FakeGroupOfPlanes {
	self(nLvCount_, 0_i32);
	self(validity, false);
	self(nWidth_B, 0_i32);
	self(nHeight_B, 0_i32);
	self(yRatioUV_B, 0_i32);
	self(planes, static_cast<FakePlaneOfBlocks **>(nullptr));
public:
	FakeGroupOfPlanes() = default;
	FakeGroupOfPlanes(std::int32_t nBlkSizeX, std::int32_t nBlkSizeY, std::int32_t nLevelCount, std::int32_t nPel, std::int32_t nOverlapX, std::int32_t nOverlapY, std::int32_t _yRatioUV, std::int32_t _nBlkX, std::int32_t _nBlkY) {
		using PlaneType = std::decay_t<decltype(*planes)>;
		nLvCount_ = nLevelCount;
		nWidth_B = (nBlkSizeX - nOverlapX) * _nBlkX + nOverlapX;
		nHeight_B = (nBlkSizeY - nOverlapY) * _nBlkY + nOverlapY;
		yRatioUV_B = _yRatioUV;
		planes = new PlaneType[nLevelCount];
		planes[0] = new FakePlaneOfBlocks{ nBlkSizeX, nBlkSizeY, 0, nPel, nOverlapX, nOverlapY, _nBlkX, _nBlkY };
		for (auto i = 1; i < nLevelCount; ++i) {
			auto nBlkX1 = ((nWidth_B >> i) - nOverlapX) / (nBlkSizeX - nOverlapX);
			auto nBlkY1 = ((nHeight_B >> i) - nOverlapY) / (nBlkSizeY - nOverlapY);
			planes[i] = new FakePlaneOfBlocks{ nBlkSizeX, nBlkSizeY, i, 1, nOverlapX, nOverlapY, nBlkX1, nBlkY1 };
		}
	}
	auto &operator=(FakeGroupOfPlanes &&OtherFakeGroupOfPlanes) {
		if (this != &OtherFakeGroupOfPlanes) {
			nLvCount_ = OtherFakeGroupOfPlanes.nLvCount_;
			validity = OtherFakeGroupOfPlanes.validity;
			nWidth_B = OtherFakeGroupOfPlanes.nWidth_B;
			nHeight_B = OtherFakeGroupOfPlanes.nHeight_B;
			yRatioUV_B = OtherFakeGroupOfPlanes.yRatioUV_B;
			std::swap(planes, OtherFakeGroupOfPlanes.planes);
		}
		return *this;
	}
	auto &operator=(const FakeGroupOfPlanes &) = delete;
	FakeGroupOfPlanes(FakeGroupOfPlanes &&OtherFakeGroupOfPlanes) {
		*this = std::move(OtherFakeGroupOfPlanes);
	}
	FakeGroupOfPlanes(const FakeGroupOfPlanes &) = delete;
	~FakeGroupOfPlanes() {
		if (planes != nullptr)
			for (auto i = 0; i < nLvCount_; ++i)
				delete planes[i];
		delete[] planes;
		planes = nullptr;
	}
	auto Update(const std::int32_t *VectorStream) {
		constexpr auto StreamHeaderOffset = 2;
		auto StreamCursor = VectorStream + StreamHeaderOffset;
		auto GetValidity = [&]() {
			return VectorStream[1] == 1;
		};
		auto UpdateVectorsForEachLevel = [&](auto Level) {
			constexpr auto LevelHeaderOffset = 1;
			auto LevelLength = StreamCursor[0];
			auto CalibratedStreamCursor = reinterpret_cast<const VectorStructure *>(StreamCursor + LevelHeaderOffset);
			planes[Level]->Update(CalibratedStreamCursor);
			StreamCursor += LevelLength;
		};
		validity = GetValidity();
		for (auto Level = nLvCount_ - 1; Level >= 0; --Level)
			UpdateVectorsForEachLevel(Level);
	}
	auto IsSceneChange(double nThSCD1, double nThSCD2) const {
		return planes[0]->IsSceneChange(nThSCD1, nThSCD2);
	}
	const auto &operator[](std::ptrdiff_t i) const {
		auto plane = planes[i];
		return *plane;
	}
	auto IsValid() const {
		return validity;
	}
	auto GetPitch() const {
		return nWidth_B;
	}
	auto GetPitchUV() const {
		return nWidth_B / 2;
	}
};