#pragma once
#include <cstdint>

using LUMAFunction = auto(*)(const std::uint8_t *, std::intptr_t)->double;

template<int nBlkWidth, int nBlkHeight>
auto Luma_C(const std::uint8_t *pSrc8, std::intptr_t nSrcPitch) {
	auto meanLuma = 0.;
	for (auto j = 0; j < nBlkHeight; ++j) {
		for (auto i = 0; i < nBlkWidth; ++i) {
			auto pSrc = reinterpret_cast<const float *>(pSrc8);
			meanLuma += pSrc[i];
		}
		pSrc8 += nSrcPitch;
	}
	return meanLuma;
}