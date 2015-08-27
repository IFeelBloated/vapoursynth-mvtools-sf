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

inline double SATDABS(double x) { return (x < 0.0) ? -x : x; }

template <typename PixelType>
float Real_Satd_4x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	double tmp[4][4];
	double a0, a1, a2, a3, s01, s23, d01, d23;
	double sum = 0.0;

	for (int i = 0; i < 4; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = pSrc[0] - pRef[0];
		a1 = pSrc[1] - pRef[1];
		a2 = pSrc[2] - pRef[2];
		a3 = pSrc[3] - pRef[3];

		s01 = a0 + a1; s23 = a2 + a3;
		d01 = a0 - a1; d23 = a2 - a3;

		tmp[i][0] = s01 + s23;
		tmp[i][1] = s01 - s23;
		tmp[i][2] = d01 - d23;
		tmp[i][3] = d01 + d23;

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}

	for (int i = 0; i < 4; i++) {
		s01 = tmp[0][i] + tmp[1][i]; s23 = tmp[2][i] + tmp[3][i];
		d01 = tmp[0][i] - tmp[1][i]; d23 = tmp[2][i] - tmp[3][i];

		sum += SATDABS(s01 + s23) + SATDABS(s01 - s23) + SATDABS(d01 - d23) + SATDABS(d01 + d23);
	}

	return (float)sum;
}

template <typename PixelType>
float Satd_4x4_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_4x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType>
float Real_Satd_8x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	double tmp[8][4];
	double a0, a1, a2, a3, a4, a5, a6, a7, s01, s23, s45, s67, d01, d23, d45, d67;
	double sum = 0.0;

	for (int i = 0; i < 8; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = pSrc[0] - pRef[0];
		a1 = pSrc[1] - pRef[1];
		a2 = pSrc[2] - pRef[2];
		a3 = pSrc[3] - pRef[3];
		a4 = pSrc[4] - pRef[4];
		a5 = pSrc[5] - pRef[5];
		a6 = pSrc[6] - pRef[6];
		a7 = pSrc[7] - pRef[7];

		s01 = a0 + a1; s23 = a2 + a3; s45 = a4 + a5; s67 = a6 + a7;
		d01 = a0 - a1; d23 = a2 - a3; d45 = a4 - a5; d67 = a6 - a7;

		tmp[i][0] = s01 + s23 + s45 + s67;
		tmp[i][1] = s01 - s23 + s45 - s67;
		tmp[i][2] = d01 - d23 + d45 - d67;
		tmp[i][3] = d01 + d23 + d45 + d67;

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (int i = 0; i < 4; i++) {
		s01 = tmp[0][i] + tmp[1][i]; s23 = tmp[2][i] + tmp[3][i]; s45 = tmp[4][i] + tmp[5][i]; s67 = tmp[6][i] + tmp[7][i];
		d01 = tmp[0][i] - tmp[1][i]; d23 = tmp[2][i] - tmp[3][i]; d45 = tmp[4][i] - tmp[5][i]; d67 = tmp[6][i] - tmp[7][i];

		sum += SATDABS(s01 + s23) + SATDABS(s45 + s67) + SATDABS(s01 - s23) + SATDABS(s45 - s67) + SATDABS(d01 - d23) + SATDABS(d45 - d67) + SATDABS(d01 + d23) + SATDABS(d45 + d67);
	}

	return (float)sum;
}

template <typename PixelType>
float Satd_8x4_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_8x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
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
