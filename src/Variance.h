#ifndef __VARIANCE_H__
#define __VARIANCE_H__
#include <cstdint>

typedef double (*LUMAFunction)(const uint8_t *pSrc, intptr_t nSrcPitch);

template<int32_t nBlkWidth, int32_t nBlkHeight, typename PixelType>
double Luma_C(const uint8_t *pSrc8, intptr_t nSrcPitch) {
	double meanLuma = 0.;
	for (int32_t j = 0; j < nBlkHeight; j++) {
		for (int32_t i = 0; i < nBlkWidth; i++) {
			const PixelType *pSrc = reinterpret_cast<const PixelType *>(pSrc8);
			meanLuma += static_cast<double>(pSrc[i]);
		}
		pSrc8 += nSrcPitch;
	}
	return meanLuma;
}

#endif
