#pragma once
#include <cstddef>
#include <utility>
#include "FakeBlockData.hpp"
#include "Cosmetics.hpp"
#include "CommonFunctions.h"
#include "SADFunctions.hpp"

class FakePlaneOfBlocks final {
	self(nWidth_Bi, 0_i32);
	self(nHeight_Bi, 0_i32);
	self(nBlkX, 0_i32);
	self(nBlkY, 0_i32);
	self(nBlkSizeX, 0_i32);
	self(nBlkSizeY, 0_i32);
	self(nBlkCount, 0_i32);
	self(nPel, 0_i32);
	self(nLogPel, 0_i32);
	self(nScale, 0_i32);
	self(nLogScale, 0_i32);
	self(nOverlapX, 0_i32);
	self(nOverlapY, 0_i32);
	self(blocks, static_cast<FakeBlockData *>(nullptr));
public:
	FakePlaneOfBlocks() = default;
	FakePlaneOfBlocks(std::int32_t sizeX, std::int32_t sizeY, std::int32_t lv, std::int32_t pel, std::int32_t _nOverlapX, std::int32_t _nOverlapY, std::int32_t _nBlkX, std::int32_t _nBlkY) {
		nBlkSizeX = sizeX;
		nBlkSizeY = sizeY;
		nOverlapX = _nOverlapX;
		nOverlapY = _nOverlapY;
		nBlkX = _nBlkX;
		nBlkY = _nBlkY;
		nWidth_Bi = nOverlapX + nBlkX * (nBlkSizeX - nOverlapX);
		nHeight_Bi = nOverlapY + nBlkY * (nBlkSizeY - nOverlapY);
		nBlkCount = nBlkX * nBlkY;
		nPel = pel;
		nLogPel = ilog2(nPel);
		nLogScale = lv;
		nScale = iexp2(nLogScale);
		blocks = new FakeBlockData[nBlkCount];
		for (auto j = 0, blkIdx = 0; j < nBlkY; ++j)
			for (auto i = 0; i < nBlkX; ++i, ++blkIdx)
				blocks[blkIdx] = { i * (nBlkSizeX - nOverlapX), j * (nBlkSizeY - nOverlapY) };
	}
	auto &operator=(const FakePlaneOfBlocks &) = delete;
	auto &operator=(FakePlaneOfBlocks &&OtherFakePlaneOfBlocks) {
		if (this != &OtherFakePlaneOfBlocks) {
			nWidth_Bi = OtherFakePlaneOfBlocks.nWidth_Bi;
			nHeight_Bi = OtherFakePlaneOfBlocks.nHeight_Bi;
			nBlkX = OtherFakePlaneOfBlocks.nBlkX;
			nBlkY = OtherFakePlaneOfBlocks.nBlkY;
			nBlkSizeX = OtherFakePlaneOfBlocks.nBlkSizeX;
			nBlkSizeY = OtherFakePlaneOfBlocks.nBlkSizeY;
			nBlkCount = OtherFakePlaneOfBlocks.nBlkCount;
			nPel = OtherFakePlaneOfBlocks.nPel;
			nLogPel = OtherFakePlaneOfBlocks.nLogPel;
			nScale = OtherFakePlaneOfBlocks.nScale;
			nLogScale = OtherFakePlaneOfBlocks.nLogScale;
			nOverlapX = OtherFakePlaneOfBlocks.nOverlapX;
			nOverlapY = OtherFakePlaneOfBlocks.nOverlapY;
			std::swap(blocks, OtherFakePlaneOfBlocks.blocks);
		}
		return *this;
	}
	FakePlaneOfBlocks(const FakePlaneOfBlocks &) = delete;
	FakePlaneOfBlocks(FakePlaneOfBlocks &&OtherFakePlaneOfBlocks) {
		*this = std::move(OtherFakePlaneOfBlocks);
	}
	~FakePlaneOfBlocks() {
		delete[] blocks;
	}
	auto Update(const void *VectorStream) {
		auto StreamCursor = reinterpret_cast<const VectorStructure *>(VectorStream);
		for (auto i = 0; i < nBlkCount; ++i) {
			blocks[i].Update(StreamCursor);
			StreamCursor += 1;
		}
	}
	auto IsSceneChange(double nTh1, double nTh2) const {
		auto sum = 0.;
		for (auto i = 0; i < nBlkCount; ++i)
			if (blocks[i].GetSAD() > nTh1)
				sum += 1.;
		return sum > nTh2;
	}
	auto IsInFrame(int i) const {
		return i >= 0 && i < nBlkCount;
	}
	const auto &operator[](std::ptrdiff_t pos) const {
		return blocks[pos];
	}
	auto GetBlockCount() const {
		return nBlkCount;
	}
	auto GetReducedWidth() const {
		return nBlkX;
	}
	auto GetReducedHeight() const {
		return nBlkY;
	}
	auto GetWidth() const {
		return nWidth_Bi;
	}
	auto GetHeight() const {
		return nHeight_Bi;
	}
	auto GetScaleLevel() const {
		return nLogScale;
	}
	auto GetEffectiveScale() const {
		return nScale;
	}
	auto GetBlockSizeX() const {
		return nBlkSizeX;
	}
	auto GetBlockSizeY() const {
		return nBlkSizeY;
	}
	auto GetPel() const {
		return nPel;
	}
	auto GetOverlapX() const {
		return nOverlapX;
	}
	auto GetOverlapY() const {
		return nOverlapY;
	}
};

