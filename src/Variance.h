#ifndef __VARIANCE_H__
#define __VARIANCE_H__

#include <cstdint>


typedef float (*LUMAFunction)(const uint8_t *pSrc, intptr_t nSrcPitch);


template<int nBlkWidth, int nBlkHeight, typename PixelType>
float Luma_C(const uint8_t *pSrc8, intptr_t nSrcPitch)
{
    float meanLuma = 0.f;
    for ( int j = 0; j < nBlkHeight; j++ )
    {
        for ( int i = 0; i < nBlkWidth; i++ ) {
            const PixelType *pSrc = (const PixelType *)pSrc8;
            meanLuma += pSrc[i];
        }
        pSrc8 += nSrcPitch;
    }
	return meanLuma;
}


#endif
