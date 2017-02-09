#pragma once
#include <cstdint>

typedef auto (*LUMAFunction)(const uint8_t *, intptr_t)->double;

template<int nBlkWidth, int nBlkHeight, typename PixelType>
auto Luma_C(const uint8_t *pSrc8, intptr_t nSrcPitch) {
	auto meanLuma = 0.;
	for (auto j = 0; j < nBlkHeight; ++j) {
		for (auto i = 0; i < nBlkWidth; ++i) {
			auto pSrc = reinterpret_cast<const PixelType *>(pSrc8);
			meanLuma += pSrc[i];
		}
		pSrc8 += nSrcPitch;
	}
	return meanLuma;
}