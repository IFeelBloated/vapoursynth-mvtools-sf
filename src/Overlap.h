#ifndef __OVERLAP__
#define __OVERLAP__

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

#include <cmath>
#include <cstdint>

class OverlapWindows {
	int nx;
	int ny;
	int ox;
	int oy;
	int size;
	short * Overlap9Windows;
	float *fWin1UVx;
	float *fWin1UVxfirst;
	float *fWin1UVxlast;
	float *fWin1UVy;
	float *fWin1UVyfirst;
	float *fWin1UVylast;
public:
	OverlapWindows(int _nx, int _ny, int _ox, int _oy);
	~OverlapWindows();
	inline int Getnx() const { return nx; }
	inline int Getny() const { return ny; }
	inline int GetSize() const { return size; }
	inline short *GetWindow(int i) const { return Overlap9Windows + size*i; }
};

typedef void(*OverlapsFunction)(uint8_t *pDst, intptr_t nDstPitch,
	const uint8_t *pSrc, intptr_t nSrcPitch,
	short *pWin, intptr_t nWinPitch);

template <int blockWidth, int blockHeight, typename PixelType2, typename PixelType>
void Overlaps_C(uint8_t *pDst8, intptr_t nDstPitch, const uint8_t *pSrc8, intptr_t nSrcPitch, short *pWin, intptr_t nWinPitch) {
	for (int j = 0; j<blockHeight; j++) {
		for (int i = 0; i<blockWidth; i++) {
			PixelType2 *pDst = (PixelType2 *)pDst8;
			const PixelType *pSrc = (const PixelType *)pSrc8;
			pDst[i] += ((pSrc[i] * pWin[i]) / 64);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
		pWin += nWinPitch;
	}
}

typedef void(*ToPixelsFunction)(uint8_t *pDst, int nDstPitch,
	const uint8_t *pSrc, int nSrcPitch,
	int width, int height);

template <typename PixelType2, typename PixelType>
void ToPixels(uint8_t *pDst8, int nDstPitch, const uint8_t *pSrc8, int nSrcPitch, int nWidth, int nHeight) {
	for (int h = 0; h<nHeight; h++) {
		for (int i = 0; i<nWidth; i++) {
			const PixelType2 *pSrc = (const PixelType2 *)pSrc8;
			PixelType *pDst = (PixelType *)pDst8;
			float a = float(pSrc[i] / 32);
			pDst[i] = min(1.f, a);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
	}
}

#endif
