#ifndef __INTERPOL__
#define __INTERPOL__

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <cstdint>

template <typename PixelType>
void VerticalBilinear(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;

	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < nHeight - 1; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = (pSrc[i] + pSrc[i + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t i = 0; i < nWidth; i++)
		pDst[i] = pSrc[i];
}

template <typename PixelType>
void HorizontalBilinear(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;

	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < nHeight; j++) {
		for (int32_t i = 0; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1]) / 2;

		pDst[nWidth - 1] = pSrc[nWidth - 1];
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void DiagonalBilinear(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;

	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < nHeight - 1; j++) {
		for (int32_t i = 0; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;

		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t i = 0; i < nWidth - 1; i++)
		pDst[i] = (pSrc[i] + pSrc[i + 1]) / 2;
	pDst[nWidth - 1] = pSrc[nWidth - 1];
}

template <typename PixelType>
void RB2F_C(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;

	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t y = 0; y < nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x * 2] + pSrc[x * 2 + 1] + pSrc[x * 2 + nSrcPitch + 1] + pSrc[x * 2 + nSrcPitch]) / 4;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
}

template <typename PixelType>
void RB2FilteredVertical(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t y = 0; y < 1; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = 1; y < nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x - nSrcPitch] + pSrc[x] * 2 + pSrc[x + nSrcPitch]) / 4;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
}

template <typename PixelType>
void RB2FilteredHorizontalInplace(uint8_t *pSrc8, int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pSrc = (PixelType *)pSrc8;
	nSrcPitch /= sizeof(PixelType);

	for (int32_t y = 0; y < nHeight; y++) {
		int32_t x = 0;
		float pSrc0 = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		for (x = 1; x < nWidth; x++)
			pSrc[x] = (pSrc[x * 2 - 1] + pSrc[x * 2] * 2 + pSrc[x * 2 + 1]) / 4;
		pSrc[0] = pSrc0;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void RB2Filtered(uint8_t *pDst, const uint8_t *pSrc, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	RB2FilteredVertical<PixelType>(pDst, pSrc, nDstPitch, nSrcPitch, nWidth * 2, nHeight);
	RB2FilteredHorizontalInplace<PixelType>(pDst, nDstPitch, nWidth, nHeight);
}

template <typename PixelType>
void RB2BilinearFilteredVertical(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	int32_t nWidthMMX = (nWidth / 8) * 8;

	for (int32_t y = 0; y < 1 && y<nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = 1; y < nHeight - 1; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x - nSrcPitch] + pSrc[x] * 3 + pSrc[x + nSrcPitch] * 3 + pSrc[x + nSrcPitch * 2]) / 8;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = max(nHeight - 1, 1); y < nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
}

template <typename PixelType>
void RB2BilinearFilteredHorizontalInplace(uint8_t *pSrc8, int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pSrc = (PixelType *)pSrc8;
	nSrcPitch /= sizeof(PixelType);

	int32_t nWidthMMX = 1 + ((nWidth - 2) / 8) * 8;

	for (int32_t y = 0; y < nHeight; y++) {
		int32_t x = 0;
		float pSrc0 = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		for (x = 1; x < nWidth - 1; x++)
			pSrc[x] = (pSrc[x * 2 - 1] + pSrc[x * 2] * 3 + pSrc[x * 2 + 1] * 3 + pSrc[x * 2 + 2]) / 8;
		pSrc[0] = pSrc0;
		for (x = max(nWidth - 1, 1); x < nWidth; x++)
			pSrc[x] = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void RB2BilinearFiltered(uint8_t *pDst, const uint8_t *pSrc, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	RB2BilinearFilteredVertical<PixelType>(pDst, pSrc, nDstPitch, nSrcPitch, nWidth * 2, nHeight);
	RB2BilinearFilteredHorizontalInplace<PixelType>(pDst, nDstPitch, nWidth, nHeight);
}

template <typename PixelType>
void RB2QuadraticVertical(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	int32_t nWidthMMX = (nWidth / 8) * 8;

	for (int32_t y = 0; y < 1 && y<nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = 1; y < nHeight - 1; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x - nSrcPitch * 2] + pSrc[x - nSrcPitch] * 9 + pSrc[x] * 22 +
				pSrc[x + nSrcPitch] * 22 + pSrc[x + nSrcPitch * 2] * 9 + pSrc[x + nSrcPitch * 3]) / 64;

		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = max(nHeight - 1, 1); y < nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
}

template <typename PixelType>
void RB2QuadraticHorizontalInplace(uint8_t *pSrc8, int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pSrc = (PixelType *)pSrc8;
	nSrcPitch /= sizeof(PixelType);

	int32_t nWidthMMX = 1 + ((nWidth - 2) / 8) * 8;

	for (int32_t y = 0; y < nHeight; y++) {
		int32_t x = 0;
		float pSrc0 = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		for (x = 1; x < nWidth - 1; x++)
			pSrc[x] = (pSrc[x * 2 - 2] + pSrc[x * 2 - 1] * 9 + pSrc[x * 2] * 22 + pSrc[x * 2 + 1] * 22 + pSrc[x * 2 + 2] * 9 + pSrc[x * 2 + 3]) / 64;
		pSrc[0] = pSrc0;
		for (x = max(nWidth - 1, 1); x < nWidth; x++)
			pSrc[x] = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void RB2Quadratic(uint8_t *pDst, const uint8_t *pSrc, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	RB2QuadraticVertical<PixelType>(pDst, pSrc, nDstPitch, nSrcPitch, nWidth * 2, nHeight);
	RB2QuadraticHorizontalInplace<PixelType>(pDst, nDstPitch, nWidth, nHeight);
}

template <typename PixelType>
void RB2CubicVertical(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	int32_t nWidthMMX = (nWidth / 8) * 8;

	for (int32_t y = 0; y < 1 && y<nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = 1; y < nHeight - 1; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x - nSrcPitch * 2] + pSrc[x - nSrcPitch] * 5 + pSrc[x] * 10 +
				pSrc[x + nSrcPitch] * 10 + pSrc[x + nSrcPitch * 2] * 5 + pSrc[x + nSrcPitch * 3]) / 32;

		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
	for (int32_t y = max(nHeight - 1, 1); y < nHeight; y++) {
		for (int32_t x = 0; x < nWidth; x++)
			pDst[x] = (pSrc[x] + pSrc[x + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch * 2;
	}
}

template <typename PixelType>
void RB2CubicHorizontalInplace(uint8_t *pSrc8, int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pSrc = (PixelType *)pSrc8;
	nSrcPitch /= sizeof(PixelType);

	int32_t nWidthMMX = 1 + ((nWidth - 2) / 8) * 8;

	for (int32_t y = 0; y < nHeight; y++) {
		int32_t x = 0;
		float pSrcw0 = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		for (x = 1; x < nWidth - 1; x++)
			pSrc[x] = (pSrc[x * 2 - 2] + pSrc[x * 2 - 1] * 5 + pSrc[x * 2] * 10 + pSrc[x * 2 + 1] * 10 + pSrc[x * 2 + 2] * 5 + pSrc[x * 2 + 3]) / 32;
		pSrc[0] = pSrcw0;
		for (x = max(nWidth - 1, 1); x < nWidth; x++)
			pSrc[x] = (pSrc[x * 2] + pSrc[x * 2 + 1]) / 2;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void RB2Cubic(uint8_t *pDst, const uint8_t *pSrc, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	RB2CubicVertical<PixelType>(pDst, pSrc, nDstPitch, nSrcPitch, nWidth * 2, nHeight);
	RB2CubicHorizontalInplace<PixelType>(pDst, nDstPitch, nWidth, nHeight);
}

template <typename PixelType>
void VerticalWiener(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < 2; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = (pSrc[i] + pSrc[i + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = 2; j < nHeight - 4; j++) {
		for (int32_t i = 0; i < nWidth; i++)
		{
			pDst[i] = min(1.f, max(0.f,
				((pSrc[i - nSrcPitch * 2])
					+ (-(pSrc[i - nSrcPitch]) + (pSrc[i] * 4) + (pSrc[i + nSrcPitch] * 4) - (pSrc[i + nSrcPitch * 2])) * 5
					+ (pSrc[i + nSrcPitch * 3])) / 32));
		}
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = nHeight - 4; j < nHeight - 1; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = (pSrc[i] + pSrc[i + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t i = 0; i < nWidth; i++)
		pDst[i] = pSrc[i];
}

template <typename PixelType>
void HorizontalWiener(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < nHeight; j++) {
		pDst[0] = (pSrc[0] + pSrc[1]) / 2;
		pDst[1] = (pSrc[1] + pSrc[2]) / 2;
		for (int32_t i = 2; i < nWidth - 4; i++) {
			pDst[i] = min(1.f, max(0.f, ((pSrc[i - 2]) + (-(pSrc[i - 1]) + (pSrc[i] * 4)
				+ (pSrc[i + 1] * 4) - (pSrc[i + 2])) * 5 + (pSrc[i + 3])) / 32));
		}
		for (int32_t i = nWidth - 4; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1]) / 2;

		pDst[nWidth - 1] = pSrc[nWidth - 1];
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void DiagonalWiener(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < 2; j++) {
		for (int32_t i = 0; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = 2; j < nHeight - 4; j++) {
		for (int32_t i = 0; i < 2; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		for (int32_t i = 2; i < nWidth - 4; i++) {
			pDst[i] = min(1.f, max(0.f,
				((pSrc[i - 2 - nSrcPitch * 2]) + (-(pSrc[i - 1 - nSrcPitch]) + (pSrc[i] * 4)
					+ (pSrc[i + 1 + nSrcPitch] * 4) - (pSrc[i + 2 + nSrcPitch * 2] * 4)) * 5 + (pSrc[i + 3 + nSrcPitch * 3])
					+ (pSrc[i + 3 - nSrcPitch * 2]) + (-(pSrc[i + 2 - nSrcPitch]) + (pSrc[i + 1] * 4)
						+ (pSrc[i + nSrcPitch] * 4) - (pSrc[i - 1 + nSrcPitch * 2])) * 5 + (pSrc[i - 2 + nSrcPitch * 3])
					) / 64));
		}
		for (int32_t i = nWidth - 4; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = nHeight - 4; j < nHeight - 1; j++) {
		for (int32_t i = 0; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t i = 0; i < nWidth - 1; i++)
		pDst[i] = (pSrc[i] + pSrc[i + 1]) / 2;
	pDst[nWidth - 1] = pSrc[nWidth - 1];
}

template <typename PixelType>
void VerticalBicubic(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < 1; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = (pSrc[i] + pSrc[i + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = 1; j < nHeight - 3; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = min(1.f, max(0.f,
				(-pSrc[i - nSrcPitch] - pSrc[i + nSrcPitch * 2] + (pSrc[i] + pSrc[i + nSrcPitch]) * 9) / 16));
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = nHeight - 3; j < nHeight - 1; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = (pSrc[i] + pSrc[i + nSrcPitch]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t i = 0; i < nWidth; i++)
		pDst[i] = pSrc[i];
}

template <typename PixelType>
void HorizontalBicubic(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < nHeight; j++) {
		pDst[0] = (pSrc[0] + pSrc[1]) / 2;
		for (int32_t i = 1; i < nWidth - 3; i++)
			pDst[i] = min(1.f, max(0.f,
				(-(pSrc[i - 1] + pSrc[i + 2]) + (pSrc[i] + pSrc[i + 1]) * 9) / 16));
		for (int32_t i = nWidth - 3; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1]) / 2;
		pDst[nWidth - 1] = pSrc[nWidth - 1];
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
}

template <typename PixelType>
void DiagonalBicubic(uint8_t *pDst8, const uint8_t *pSrc8, int32_t nDstPitch,
	int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc = (PixelType *)pSrc8;
	nDstPitch /= sizeof(PixelType);
	nSrcPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < 1; j++) {
		for (int32_t i = 0; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = 1; j < nHeight - 3; j++) {
		for (int32_t i = 0; i < 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		for (int32_t i = 1; i < nWidth - 3; i++)
			pDst[i] = min(1.f, max(0.f,
				(-pSrc[i - 1 - nSrcPitch] - pSrc[i + 2 + nSrcPitch * 2] + (pSrc[i] + pSrc[i + 1 + nSrcPitch]) * 9
					- pSrc[i - 1 + nSrcPitch * 2] - pSrc[i + 2 - nSrcPitch] + (pSrc[i + 1] + pSrc[i + nSrcPitch]) * 9
					) / 32));
		for (int32_t i = nWidth - 3; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t j = nHeight - 3; j < nHeight - 1; j++) {
		for (int32_t i = 0; i < nWidth - 1; i++)
			pDst[i] = (pSrc[i] + pSrc[i + 1] + pSrc[i + nSrcPitch] + pSrc[i + nSrcPitch + 1]) / 4;
		pDst[nWidth - 1] = (pSrc[nWidth - 1] + pSrc[nWidth + nSrcPitch - 1]) / 2;
		pDst += nDstPitch;
		pSrc += nSrcPitch;
	}
	for (int32_t i = 0; i < nWidth - 1; i++)
		pDst[i] = (pSrc[i] + pSrc[i + 1]) / 2;
	pDst[nWidth - 1] = pSrc[nWidth - 1];
}

template <typename PixelType>
void Average2(uint8_t *pDst8, const uint8_t *pSrc18, const uint8_t *pSrc28,
	int32_t nPitch, int32_t nWidth, int32_t nHeight) {
	PixelType *pDst = (PixelType *)pDst8;
	PixelType *pSrc1 = (PixelType *)pSrc18;
	PixelType *pSrc2 = (PixelType *)pSrc28;
	nPitch /= sizeof(PixelType);

	for (int32_t j = 0; j < nHeight; j++) {
		for (int32_t i = 0; i < nWidth; i++)
			pDst[i] = (pSrc1[i] + pSrc2[i]) / 2;
		pDst += nPitch;
		pSrc1 += nPitch;
		pSrc2 += nPitch;
	}
}

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif
