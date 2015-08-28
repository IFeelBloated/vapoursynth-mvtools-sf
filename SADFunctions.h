// Functions that computes distances between blocks

// See legal notice in Copying.txt for more information

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .

#ifndef __SAD_FUNC__
#define __SAD_FUNC__

#include <cstdint>


typedef float (*SADFunction)(const uint8_t *pSrc, intptr_t nSrcPitch,
        const uint8_t *pRef, intptr_t nRefPitch);

inline float SADABS(float x) { return (x < 0.f) ? -x : x; }

template<int nBlkWidth, int nBlkHeight, typename PixelType>
float Sad_C(const uint8_t *pSrc8, intptr_t nSrcPitch,const uint8_t *pRef8,
        intptr_t nRefPitch)
{
    float sum = 0.f;
    for ( int y = 0; y < nBlkHeight; y++ )
    {
        for ( int x = 0; x < nBlkWidth; x++ ) {
            const PixelType *pSrc = (const PixelType *)pSrc8;
            const PixelType *pRef = (const PixelType *)pRef8;
            sum += SADABS(pSrc[x] - pRef[x]);
        }
        pSrc8 += nSrcPitch;
        pRef8 += nRefPitch;
    }
	return sum;
}

#define HADAMARD4(d0, d1, d2, d3, s0, s1, s2, s3) {\
    SumType2 t0 = s0 + s1;\
    SumType2 t1 = s0 - s1;\
    SumType2 t2 = s2 + s3;\
    SumType2 t3 = s2 - s3;\
    d0 = t0 + t2;\
    d2 = t0 - t2;\
    d1 = t1 + t3;\
    d3 = t1 - t3;\
}

template <typename SumType, typename SumType2>
static inline SumType2 abs2(SumType2 a)
{
	int bitsPerSum = 8 * sizeof(SumType);

	SumType2 s = ((a >> (bitsPerSum - 1))&(((SumType2)1 << bitsPerSum) + 1))*((SumType)-1);
	return (a + s) ^ s;
}

template <typename PixelType, typename SumType, typename SumType2>
float Real_Satd_4x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	int bitsPerSum = 8 * sizeof(SumType);

	SumType2 tmp[4][2];
	SumType2 a0, a1, a2, a3, b0, b1;
	SumType2 sum = 0;

	for (int i = 0; i < 4; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = short (65536 * (pSrc[0] - pRef[0]));
		a1 = short (65536 * (pSrc[1] - pRef[1]));
		b0 = (a0 + a1) + ((a0 - a1) << bitsPerSum);
		a2 = short (65536 * (pSrc[2] - pRef[2]));
		a3 = short (65536 * (pSrc[3] - pRef[3]));
		b1 = (a2 + a3) + ((a2 - a3) << bitsPerSum);
		tmp[i][0] = b0 + b1;
		tmp[i][1] = b0 - b1;

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}

	for (int i = 0; i < 2; i++) {
		HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
		a0 = abs2<SumType, SumType2>(a0) +abs2<SumType, SumType2>(a1) +abs2<SumType, SumType2>(a2) +abs2<SumType, SumType2>(a3);
		sum += ((SumType)a0) + (a0 >> bitsPerSum);
	}

	return float ( double (sum >> 1) / 65536);
}

template <typename PixelType>
float Satd_4x4_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
		return Real_Satd_4x4_C<PixelType, uint32_t, uint64_t>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType, typename SumType2>
float Real_Satd_8x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	int bitsPerSum = 8 * sizeof(SumType);

	SumType2 tmp[4][4];
	SumType2 a0, a1, a2, a3;
	SumType2 sum = 0;

	for (int i = 0; i < 4; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = short (65536 * ((pSrc[0] - pRef[0]))) + ((SumType2)(short (65536 * (pSrc[4] - pRef[4]))) << bitsPerSum);
		a1 = short (65536 * ((pSrc[1] - pRef[1]))) + ((SumType2)(short (65536 * (pSrc[5] - pRef[5]))) << bitsPerSum);
		a2 = short (65536 * ((pSrc[2] - pRef[2]))) + ((SumType2)(short (65536 * (pSrc[6] - pRef[6]))) << bitsPerSum);
		a3 = short (65536 * ((pSrc[3] - pRef[3]))) + ((SumType2)(short (65536 * (pSrc[7] - pRef[7]))) << bitsPerSum);
		HADAMARD4(tmp[i][0], tmp[i][1], tmp[i][2], tmp[i][3], a0, a1, a2, a3);

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (int i = 0; i < 4; i++) {
		HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
		sum += abs2<SumType, SumType2>(a0) +abs2<SumType, SumType2>(a1) +abs2<SumType, SumType2>(a2) +abs2<SumType, SumType2>(a3);
	}

	return float (double (((((SumType)sum) + (sum >> bitsPerSum)) >> 1)) / 65536);
}

template <typename PixelType>
float Satd_8x4_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
		return Real_Satd_8x4_C<PixelType, uint32_t, uint64_t>(pSrc, nSrcPitch, pRef, nRefPitch);
}

// Only handles 4x4, 8x4, 8x8, 8x16, 16x8, and 16x16.
template <int nBlkWidth, int nBlkHeight, typename PixelType>
float Satd_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	if (nBlkWidth == 4 && nBlkHeight == 4)
		return Satd_4x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 8 && nBlkHeight == 4)
		return Satd_8x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else {
		int bytesPerSample = sizeof(PixelType);

		float sum = Satd_8x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch)
			+ Satd_8x4_C<PixelType>(pSrc + 4 * nSrcPitch, nSrcPitch, pRef + 4 * nRefPitch, nRefPitch);

		if (nBlkWidth == 16)
			sum += Satd_8x4_C<PixelType>(pSrc + 8 * bytesPerSample, nSrcPitch, pRef + 8 * bytesPerSample, nRefPitch)
			+ Satd_8x4_C<PixelType>(pSrc + 8 * bytesPerSample + 4 * nSrcPitch, nSrcPitch, pRef + 8 * bytesPerSample + 4 * nSrcPitch, nRefPitch);

		if (nBlkHeight == 16)
			sum += Satd_8x4_C<PixelType>(pSrc + 8 * nSrcPitch, nSrcPitch, pRef + 8 * nRefPitch, nRefPitch)
			+ Satd_8x4_C<PixelType>(pSrc + 12 * nSrcPitch, nSrcPitch, pRef + 12 * nRefPitch, nRefPitch);

		if (nBlkWidth == 16 && nBlkHeight == 16)
			sum += Satd_8x4_C<PixelType>(pSrc + 8 * bytesPerSample + 8 * nSrcPitch, nSrcPitch, pRef + 8 * bytesPerSample + 8 * nRefPitch, nRefPitch)
			+ Satd_8x4_C<PixelType>(pSrc + 8 * bytesPerSample + 12 * nSrcPitch, nSrcPitch, pRef + 8 * bytesPerSample + 12 * nRefPitch, nRefPitch);

		return sum;
	}
}

#endif
