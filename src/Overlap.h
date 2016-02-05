#ifndef __OVERLAP__
#define __OVERLAP__

#include <cmath>
#include <cstdint>

#ifndef M_PI
#define M_PI       3.14159265358979323846f
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)            (((a) < (b)) ? (b) : (a))
#endif

#define OW_TL 0
#define OW_TM 1
#define OW_TR 2
#define OW_ML 3
#define OW_MM 4
#define OW_MR 5
#define OW_BL 6
#define OW_BM 7
#define OW_BR 8

class OverlapWindows {
	int32_t nx;
	int32_t ny;
	int32_t ox;
	int32_t oy;
	int32_t size;
	int16_t * Overlap9Windows;
	float *fWin1UVx;
	float *fWin1UVxfirst;
	float *fWin1UVxlast;
	float *fWin1UVy;
	float *fWin1UVyfirst;
	float *fWin1UVylast;
public:
	OverlapWindows(int32_t _nx, int32_t _ny, int32_t _ox, int32_t _oy);
	~OverlapWindows();
	inline int32_t Getnx() const { return nx; }
	inline int32_t Getny() const { return ny; }
	inline int32_t GetSize() const { return size; }
	inline int16_t *GetWindow(int32_t i) const { return Overlap9Windows + size*i; }
};

typedef void(*OverlapsFunction)(uint8_t *pDst, intptr_t nDstPitch,
	const uint8_t *pSrc, intptr_t nSrcPitch,
	int16_t *pWin, intptr_t nWinPitch);

template <int32_t blockWidth, int32_t blockHeight, typename PixelType2, typename PixelType>
void Overlaps_C(uint8_t *pDst8, intptr_t nDstPitch, const uint8_t *pSrc8, intptr_t nSrcPitch, int16_t *pWin, intptr_t nWinPitch) {
	for (int32_t j = 0; j<blockHeight; j++) {
		for (int32_t i = 0; i<blockWidth; i++) {
			PixelType2 *pDst = (PixelType2 *)pDst8;
			const PixelType *pSrc = (const PixelType *)pSrc8;
			pDst[i] += ((pSrc[i] * pWin[i]) / 64);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
		pWin += nWinPitch;
	}
}

typedef void(*ToPixelsFunction)(uint8_t *pDst, int32_t nDstPitch,
	const uint8_t *pSrc, int32_t nSrcPitch,
	int32_t width, int32_t height);

template <typename PixelType2, typename PixelType>
void ToPixels(uint8_t *pDst8, int32_t nDstPitch, const uint8_t *pSrc8, int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	for (int32_t h = 0; h<nHeight; h++) {
		for (int32_t i = 0; i<nWidth; i++) {
			const PixelType2 *pSrc = (const PixelType2 *)pSrc8;
			PixelType *pDst = (PixelType *)pDst8;
			float a = static_cast<float>(pSrc[i] / 32);
			pDst[i] = min(1.f, a);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
	}
}

#endif
