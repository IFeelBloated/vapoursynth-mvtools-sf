#pragma once
#include <cstring>
#include <cstdint>

using COPYFunction = auto(*)(std::uint8_t *, std::intptr_t, const std::uint8_t *, std::intptr_t)->void;

template<int nBlkWidth, int nBlkHeight>
auto Copy_C(std::uint8_t *pDst, std::intptr_t nDstPitch, const std::uint8_t *pSrc, std::intptr_t nSrcPitch) {
	for (auto i = 0; i < nBlkHeight; ++i) {
		std::memcpy(pDst, pSrc, nBlkWidth * sizeof(float));
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
}
