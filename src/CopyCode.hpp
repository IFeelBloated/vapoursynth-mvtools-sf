#pragma once
#include <cstring>
#include <cstdint>

using COPYFunction = auto(*)(uint8_t *, intptr_t, const uint8_t *, intptr_t)->void;

template<int nBlkWidth, int nBlkHeight, typename PixelType>
auto Copy_C(uint8_t *pDst, intptr_t nDstPitch, const uint8_t *pSrc, intptr_t nSrcPitch) {
	for (auto i = 0; i < nBlkHeight; ++i) {
		std::memcpy(pDst, pSrc, nBlkWidth * sizeof(PixelType));
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
}
