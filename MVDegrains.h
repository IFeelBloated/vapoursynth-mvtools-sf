#ifndef __MV_DEGRAINS__
#define __MV_DEGRAINS__

#include <cstdint>
#include <cstring>

#include "MVFrame.h"
#include "FakeSAD.h"

enum VectorOrder {
    Backward1 = 0,
    Forward1,
    Backward2,
    Forward2,
    Backward3,
    Forward3,
    Backward4,	
    Forward4, 
    Backward5,  
    Forward5,
    Backward6,
    Forward6
};


typedef void (*DenoiseFunction)(uint8_t *pDst, int nDstPitch, const uint8_t *pSrc, int nSrcPitch, const uint8_t **_pRefs, const int *nRefPitches, float WSrc, const float *WRefs);


// XXX Moves the pointers passed in pRefs. This is okay because they are not
// used after this function is done with them.
template <int radius, int blockWidth, int blockHeight, typename PixelType>
void Degrain_C(uint8_t *pDst8, int nDstPitch, const uint8_t *pSrc8, int nSrcPitch, const uint8_t **pRefs8, const int *nRefPitches, float WSrc, const float *WRefs) {
    for (int y = 0; y < blockHeight; y++) {
        for (int x = 0; x < blockWidth; x++) {
            const PixelType *pSrc = (const PixelType *)pSrc8;
            PixelType *pDst = (PixelType *)pDst8;

            float sum = pSrc[x] * WSrc;

            for (int r = 0; r < radius*2; r++) {
                const PixelType *pRef = (const PixelType *)pRefs8[r];
                sum += pRef[x] * WRefs[r];
            }

            pDst[x] = sum / 256;
        }

        pDst8 += nDstPitch;
        pSrc8 += nSrcPitch;
        for (int r = 0; r < radius*2; r++)
            pRefs8[r] += nRefPitches[r];
    }
}

typedef void (*LimitFunction)(uint8_t *pDst, intptr_t nDstPitch, const uint8_t *pSrc, intptr_t nSrcPitch, intptr_t nWidth, intptr_t nHeight, double nLimit);

template <typename PixelType>
static void LimitChanges_C(uint8_t *pDst8, intptr_t nDstPitch, const uint8_t *pSrc8, intptr_t nSrcPitch, intptr_t nWidth, intptr_t nHeight, double nLimit) {
    for (int h = 0; h < nHeight; h++) {
        for (int i = 0; i < nWidth; i++) {
            const PixelType *pSrc = (const PixelType *)pSrc8;
            PixelType *pDst = (PixelType *)pDst8;

            pDst[i] = (PixelType)VSMIN(VSMAX(pDst[i], (pSrc[i] - nLimit)), (pSrc[i] + nLimit));
        }
        pDst8 += nDstPitch;
        pSrc8 += nSrcPitch;
    }
}


inline float DegrainWeight(double thSAD, double blockSAD) {
    if (blockSAD >= thSAD)
        return 0.f;

    return float((thSAD - blockSAD) * (thSAD + blockSAD) * 256 / (thSAD * thSAD + blockSAD * blockSAD));
}


inline void useBlock(const uint8_t * &p, int &np, float &WRef, bool isUsable, const MVClipBalls *mvclip, int i, const MVPlane *pPlane, const uint8_t **pSrcCur, int xx, const int *nSrcPitch, int nLogPel, int plane, int xSubUV, int ySubUV, const float *thSAD) {
    if (isUsable) {
        const FakeBlockData &block = mvclip->GetBlock(0, i);
        int blx = (block.GetX() << nLogPel) + block.GetMV().x;
        int bly = (block.GetY() << nLogPel) + block.GetMV().y;
        p = pPlane->GetPointer(plane ? blx >> xSubUV : blx, plane ? bly >> ySubUV : bly);
        np = pPlane->GetPitch();
        float blockSAD = Back2FLT (block.GetSAD());
        WRef = DegrainWeight(thSAD[plane], blockSAD);
    } else {
        p = pSrcCur[plane] + xx;
        np = nSrcPitch[plane];
        WRef = 0.f;
    }
}


template <int radius>
static inline void normalizeWeights(float &WSrc, float *WRefs) {
    // normalize weights to 256
    WSrc = 256.f;
    float WSum = WSrc + 1.f;
    for (int r = 0; r < radius*2; r++)
        WSum += WRefs[r];

    for (int r = 0; r < radius*2; r++) {
        WRefs[r] = WRefs[r] * 256 / WSum;
        WSrc -= WRefs[r];
    }
}

#endif
