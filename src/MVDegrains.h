#ifndef __MV_DEGRAINS__
#define __MV_DEGRAINS__
#include <cstdint>
#include <cstring>
#include "MVFrame.h"
#include "SADFunctions.hpp"

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
	Forward6,
	Backward7,
	Forward7,
	Backward8,
	Forward8,
	Backward9,
	Forward9,
	Backward10,
	Forward10,
	Backward11,
	Forward11,
	Backward12,
	Forward12,
	Backward13,
	Forward13,
	Backward14,
	Forward14,
	Backward15,
	Forward15,
	Backward16,
	Forward16,
	Backward17,
	Forward17,
	Backward18,
	Forward18,
	Backward19,
	Forward19,
	Backward20,
	Forward20,
	Backward21,
	Forward21,
	Backward22,
	Forward22,
	Backward23,
	Forward23,
	Backward24,
	Forward24
};

typedef void(*DenoiseFunction)(uint8_t *pDst, int32_t nDstPitch, const uint8_t *pSrc, int32_t nSrcPitch, const uint8_t **_pRefs, const int32_t *nRefPitches, double WSrc, const double *WRefs);

template <int32_t radius, int32_t blockWidth, int32_t blockHeight, typename PixelType>
void Degrain_C(uint8_t *pDst8, int32_t nDstPitch, const uint8_t *pSrc8, int32_t nSrcPitch, const uint8_t **pRefs8, const int32_t *nRefPitches, double WSrc, const double *WRefs) {
	for (int32_t y = 0; y < blockHeight; y++) {
		for (int32_t x = 0; x < blockWidth; x++) {
			const PixelType *pSrc = (const PixelType *)pSrc8;
			PixelType *pDst = (PixelType *)pDst8;
			double sum = pSrc[x] * WSrc;
			for (int32_t r = 0; r < radius * 2; r++) {
				const PixelType *pRef = (const PixelType *)pRefs8[r];
				sum += pRef[x] * WRefs[r];
			}
			pDst[x] = static_cast<PixelType>(sum / 256);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
		for (int32_t r = 0; r < radius * 2; r++)
			pRefs8[r] += nRefPitches[r];
	}
}

typedef void(*LimitFunction)(uint8_t *pDst, intptr_t nDstPitch, const uint8_t *pSrc, intptr_t nSrcPitch, intptr_t nWidth, intptr_t nHeight, double nLimit);

template <typename PixelType>
static void LimitChanges_C(uint8_t *pDst8, intptr_t nDstPitch, const uint8_t *pSrc8, intptr_t nSrcPitch, intptr_t nWidth, intptr_t nHeight, double nLimit) {
	for (int32_t h = 0; h < nHeight; h++) {
		for (int32_t i = 0; i < nWidth; i++) {
			const PixelType *pSrc = (const PixelType *)pSrc8;
			PixelType *pDst = (PixelType *)pDst8;
			pDst[i] = (PixelType)VSMIN(VSMAX(pDst[i], (pSrc[i] - nLimit)), (pSrc[i] + nLimit));
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
	}
}

inline double DegrainWeight(double thSAD, double blockSAD) {
	if (blockSAD >= thSAD)
		return 0.;
	return (thSAD - blockSAD) * (thSAD + blockSAD) * 256 / (thSAD * thSAD + blockSAD * blockSAD);
}

inline void useBlock(const uint8_t * &p, int32_t &np, double &WRef, bool isUsable, const MVClipBalls &mvclip, int32_t i, const MVPlane *pPlane, const uint8_t **pSrcCur, int32_t xx, const int32_t *nSrcPitch, int32_t nLogPel, int32_t plane, int32_t xSubUV, int32_t ySubUV, const double *thSAD) {
	if (isUsable) {
		auto &block = mvclip[0][i];
		int32_t blx = (block.GetX() << nLogPel) + block.GetMV().x;
		int32_t bly = (block.GetY() << nLogPel) + block.GetMV().y;
		p = pPlane->GetPointer(plane ? blx >> xSubUV : blx, plane ? bly >> ySubUV : bly);
		np = pPlane->GetPitch();
		double blockSAD = block.GetSAD();
		WRef = DegrainWeight(thSAD[plane], blockSAD);
	}
	else {
		p = pSrcCur[plane] + xx;
		np = nSrcPitch[plane];
		WRef = 0.;
	}
}

template <int32_t radius>
static inline void normalizeWeights(double &WSrc, double *WRefs) {
	WSrc = 256.;
	double WSum = WSrc + 1.;
	for (int32_t r = 0; r < radius * 2; r++)
		WSum += WRefs[r];
	for (int32_t r = 0; r < radius * 2; r++) {
		WRefs[r] = WRefs[r] * 256 / WSum;
		WSrc -= WRefs[r];
	}
}

#endif
