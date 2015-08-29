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

#define HADAMARD8(d0, d1, d2, d3, d4, d5, d6, d7, s0, s1, s2, s3, s4, s5, s6, s7) {\
    SumType2 t0 = s0 + s1;\
    SumType2 t1 = s0 - s1;\
    SumType2 t2 = s2 + s3;\
    SumType2 t3 = s2 - s3;\
    SumType2 t4 = s4 + s5;\
    SumType2 t5 = s4 - s5;\
    SumType2 t6 = s6 + s7;\
    SumType2 t7 = s6 - s7;\
    d0 = t0 + t4;\
    d2 = t0 - t4;\
    d1 = t1 + t5;\
    d3 = t1 - t5;\
    d4 = t2 + t6;\
    d5 = t2 - t6;\
    d6 = t3 + t7;\
    d7 = t3 - t7;\
}

#define HADAMARD16(d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15) {\
    SumType2 t0 = s0 + s1;\
    SumType2 t1 = s0 - s1;\
    SumType2 t2 = s2 + s3;\
    SumType2 t3 = s2 - s3;\
    SumType2 t4 = s4 + s5;\
    SumType2 t5 = s4 - s5;\
    SumType2 t6 = s6 + s7;\
    SumType2 t7 = s6 - s7;\
    SumType2 t8 = s8 + s9;\
    SumType2 t9 = s8 - s9;\
    SumType2 t10 = s10 + s11;\
    SumType2 t11 = s10 - s11;\
    SumType2 t12 = s12 + s13;\
    SumType2 t13 = s12 - s13;\
    SumType2 t14 = s14 + s15;\
    SumType2 t15 = s14 - s15;\
    d0  = t0 + t8;\
    d2  = t0 - t8;\
    d1  = t1 + t9;\
    d3  = t1 - t9;\
    d4  = t2 + t10;\
    d5  = t2 - t10;\
    d6  = t3 + t11;\
    d7  = t3 - t11;\
    d8  = t4 + t12;\
    d9  = t4 - t12;\
    d10 = t5 + t13;\
    d11 = t5 - t13;\
    d12 = t6 + t14;\
    d13 = t6 - t14;\
    d14 = t7 + t15;\
    d15 = t7 - t15;\
}

#define HADAMARD32(d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31) {\
    SumType2 t0 = s0 + s1;\
    SumType2 t1 = s0 - s1;\
    SumType2 t2 = s2 + s3;\
    SumType2 t3 = s2 - s3;\
    SumType2 t4 = s4 + s5;\
    SumType2 t5 = s4 - s5;\
    SumType2 t6 = s6 + s7;\
    SumType2 t7 = s6 - s7;\
    SumType2 t8 = s8 + s9;\
    SumType2 t9 = s8 - s9;\
    SumType2 t10 = s10 + s11;\
    SumType2 t11 = s10 - s11;\
    SumType2 t12 = s12 + s13;\
    SumType2 t13 = s12 - s13;\
    SumType2 t14 = s14 + s15;\
    SumType2 t15 = s14 - s15;\
    SumType2 t16 = s16 + s17;\
    SumType2 t17 = s16 - s17;\
    SumType2 t18 = s18 + s19;\
    SumType2 t19 = s18 - s19;\
    SumType2 t20 = s20 + s21;\
    SumType2 t21 = s20 - s21;\
    SumType2 t22 = s22 + s23;\
    SumType2 t23 = s22 - s23;\
    SumType2 t24 = s24 + s25;\
    SumType2 t25 = s24 - s25;\
    SumType2 t26 = s26 + s27;\
    SumType2 t27 = s26 - s27;\
    SumType2 t28 = s28 + s29;\
    SumType2 t29 = s28 - s29;\
    SumType2 t30 = s30 + s31;\
    SumType2 t31 = s30 - s31;\
    d0   = t0 + t16;\
    d2   = t0 - t16;\
    d1   = t1 + t17;\
    d3   = t1 - t17;\
    d4   = t2 + t18;\
    d5   = t2 - t18;\
    d6   = t3 + t19;\
    d7   = t3 - t19;\
    d8   = t4 + t20;\
    d9   = t4 - t20;\
    d10  = t5 + t21;\
    d11  = t5 - t21;\
    d12  = t6 + t22;\
    d13  = t6 - t22;\
    d14  = t7 + t23;\
    d15  = t7 - t23;\
    d16  = t8 + t24;\
    d17  = t8 - t24;\
    d18  = t9 + t25;\
    d19  = t9 - t25;\
    d20  = t10 + t26;\
    d21  = t10 - t26;\
    d22  = t11 + t27;\
    d23  = t11 - t27;\
    d24  = t12 + t28;\
    d25  = t12 - t28;\
    d26  = t13 + t29;\
    d27  = t13 - t29;\
    d28  = t14 + t30;\
    d29  = t14 - t30;\
    d30  = t15 + t31;\
    d31  = t15 - t31;\
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

		a0 = short (32767 * (pSrc[0] - pRef[0]) + 0.5f);
		a1 = short (32767 * (pSrc[1] - pRef[1]) + 0.5f);
		b0 = (a0 + a1) + ((a0 - a1) << bitsPerSum);
		a2 = short (32767 * (pSrc[2] - pRef[2]) + 0.5f);
		a3 = short (32767 * (pSrc[3] - pRef[3]) + 0.5f);
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

	return float (double (sum >> 1) / 32767);
}

template <typename PixelType>
float Satd_4x4_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
		return Real_Satd_4x4_C<PixelType, long, long long>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType, typename SumType2>
float Real_Satd_8x8_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	int bitsPerSum = 8 * sizeof(SumType);

	SumType2 tmp[8][4];
	SumType2 a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3;
	SumType2 sum = 0;

	for (int i = 0; i < 8; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = short(32767 * (pSrc[0] - pRef[0]) + 0.5f);
		a1 = short(32767 * (pSrc[1] - pRef[1]) + 0.5f);
		b0 = (a0 + a1) + ((a0 - a1) << bitsPerSum);
		a2 = short(32767 * (pSrc[2] - pRef[2]) + 0.5f);
		a3 = short(32767 * (pSrc[3] - pRef[3]) + 0.5f);
		b1 = (a2 + a3) + ((a2 - a3) << bitsPerSum);
		a4 = short(32767 * (pSrc[4] - pRef[4]) + 0.5f);
		a5 = short(32767 * (pSrc[5] - pRef[5]) + 0.5f);
		b2 = (a4 + a5) + ((a4 - a5) << bitsPerSum);
		a6 = short(32767 * (pSrc[6] - pRef[6]) + 0.5f);
		a7 = short(32767 * (pSrc[7] - pRef[7]) + 0.5f);
		b3 = (a6 + a7) + ((a6 - a7) << bitsPerSum);
		tmp[i][0] = b0 + b2;
		tmp[i][1] = b0 - b2;
		tmp[i][2] = b1 + b3;
		tmp[i][3] = b1 - b3;

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}

	for (int i = 0; i < 4; i++) {
		HADAMARD8(a0, a1, a2, a3, a4, a5, a6, a7, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i], tmp[4][i], tmp[5][i], tmp[6][i], tmp[7][i]);
		a0 = abs2<SumType, SumType2>(a0) +abs2<SumType, SumType2>(a1) +abs2<SumType, SumType2>(a2) +abs2<SumType, SumType2>(a3) +abs2<SumType, SumType2>(a4) +abs2<SumType, SumType2>(a5) +abs2<SumType, SumType2>(a6) +abs2<SumType, SumType2>(a7);
		sum += ((SumType)a0) + (a0 >> bitsPerSum);
	}

	return float(double(sum >> 1) / 32767);
}

template <typename PixelType>
float Satd_8x8_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_8x8_C<PixelType, long, long long>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType, typename SumType2>
float Real_Satd_16x16_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	int bitsPerSum = 8 * sizeof(SumType);

	SumType2 tmp[16][8];
	SumType2 a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, b0, b1, b2, b3, b4, b5, b6, b7;
	SumType2 sum = 0;

	for (int i = 0; i < 16; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = short(32767 * (pSrc[0] - pRef[0]) + 0.5f);
		a1 = short(32767 * (pSrc[1] - pRef[1]) + 0.5f);
		b0 = (a0 + a1) + ((a0 - a1) << bitsPerSum);
		a2 = short(32767 * (pSrc[2] - pRef[2]) + 0.5f);
		a3 = short(32767 * (pSrc[3] - pRef[3]) + 0.5f);
		b1 = (a2 + a3) + ((a2 - a3) << bitsPerSum);
		a4 = short(32767 * (pSrc[4] - pRef[4]) + 0.5f);
		a5 = short(32767 * (pSrc[5] - pRef[5]) + 0.5f);
		b2 = (a4 + a5) + ((a4 - a5) << bitsPerSum);
		a6 = short(32767 * (pSrc[6] - pRef[6]) + 0.5f);
		a7 = short(32767 * (pSrc[7] - pRef[7]) + 0.5f);
		b3 = (a6 + a7) + ((a6 - a7) << bitsPerSum);
		a8 = short(32767 * (pSrc[8] - pRef[8]) + 0.5f);
		a9 = short(32767 * (pSrc[9] - pRef[9]) + 0.5f);
		b4 = (a8 + a9) + ((a8 - a9) << bitsPerSum);
		a10 = short(32767 * (pSrc[10] - pRef[10]) + 0.5f);
		a11 = short(32767 * (pSrc[11] - pRef[11]) + 0.5f);
		b5 = (a10 + a11) + ((a10 - a11) << bitsPerSum);
		a12 = short(32767 * (pSrc[12] - pRef[12]) + 0.5f);
		a13 = short(32767 * (pSrc[13] - pRef[13]) + 0.5f);
		b6 = (a12 + a13) + ((a12 - a13) << bitsPerSum);
		a14 = short(32767 * (pSrc[14] - pRef[14]) + 0.5f);
		a15 = short(32767 * (pSrc[15] - pRef[15]) + 0.5f);
		b7 = (a14 + a15) + ((a14 - a15) << bitsPerSum);
		tmp[i][0] = b0 + b4;
		tmp[i][1] = b0 - b4;
		tmp[i][2] = b1 + b5;
		tmp[i][3] = b1 - b5;
		tmp[i][4] = b2 + b6;
		tmp[i][5] = b2 - b6;
		tmp[i][6] = b3 + b7;
		tmp[i][7] = b3 - b7;

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}

	for (int i = 0; i < 8; i++) {
		HADAMARD16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i], tmp[4][i], tmp[5][i], tmp[6][i], tmp[7][i], tmp[8][i], tmp[9][i], tmp[10][i], tmp[11][i], tmp[12][i], tmp[13][i], tmp[14][i], tmp[15][i]);
		a0 = abs2<SumType, SumType2>(a0) +abs2<SumType, SumType2>(a1) +abs2<SumType, SumType2>(a2) +abs2<SumType, SumType2>(a3) +abs2<SumType, SumType2>(a4) +abs2<SumType, SumType2>(a5) +abs2<SumType, SumType2>(a6) +abs2<SumType, SumType2>(a7) +abs2<SumType, SumType2>(a8) +abs2<SumType, SumType2>(a9) +abs2<SumType, SumType2>(a10) +abs2<SumType, SumType2>(a11) +abs2<SumType, SumType2>(a12) +abs2<SumType, SumType2>(a13) +abs2<SumType, SumType2>(a14) +abs2<SumType, SumType2>(a15);
		sum += ((SumType)a0) + (a0 >> bitsPerSum);
	}

	return float(double(sum >> 1) / 32767);
}

template <typename PixelType>
float Satd_16x16_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_16x16_C<PixelType, long, long long>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType, typename SumType2>
float Real_Satd_32x32_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	int bitsPerSum = 8 * sizeof(SumType);

	SumType2 tmp[32][16];
	SumType2 a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15;
	SumType2 sum = 0;

	for (int i = 0; i < 32; i++) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;

		a0 = short(32767 * (pSrc[0] - pRef[0]) + 0.5f);
		a1 = short(32767 * (pSrc[1] - pRef[1]) + 0.5f);
		b0 = (a0 + a1) + ((a0 - a1) << bitsPerSum);
		a2 = short(32767 * (pSrc[2] - pRef[2]) + 0.5f);
		a3 = short(32767 * (pSrc[3] - pRef[3]) + 0.5f);
		b1 = (a2 + a3) + ((a2 - a3) << bitsPerSum);
		a4 = short(32767 * (pSrc[4] - pRef[4]) + 0.5f);
		a5 = short(32767 * (pSrc[5] - pRef[5]) + 0.5f);
		b2 = (a4 + a5) + ((a4 - a5) << bitsPerSum);
		a6 = short(32767 * (pSrc[6] - pRef[6]) + 0.5f);
		a7 = short(32767 * (pSrc[7] - pRef[7]) + 0.5f);
		b3 = (a6 + a7) + ((a6 - a7) << bitsPerSum);
		a8 = short(32767 * (pSrc[8] - pRef[8]) + 0.5f);
		a9 = short(32767 * (pSrc[9] - pRef[9]) + 0.5f);
		b4 = (a8 + a9) + ((a8 - a9) << bitsPerSum);
		a10 = short(32767 * (pSrc[10] - pRef[10]) + 0.5f);
		a11 = short(32767 * (pSrc[11] - pRef[11]) + 0.5f);
		b5 = (a10 + a11) + ((a10 - a11) << bitsPerSum);
		a12 = short(32767 * (pSrc[12] - pRef[12]) + 0.5f);
		a13 = short(32767 * (pSrc[13] - pRef[13]) + 0.5f);
		b6 = (a12 + a13) + ((a12 - a13) << bitsPerSum);
		a14 = short(32767 * (pSrc[14] - pRef[14]) + 0.5f);
		a15 = short(32767 * (pSrc[15] - pRef[15]) + 0.5f);
		b7 = (a14 + a15) + ((a14 - a15) << bitsPerSum);
		a16 = short(32767 * (pSrc[16] - pRef[16]) + 0.5f);
		a17 = short(32767 * (pSrc[17] - pRef[17]) + 0.5f);
		b8 = (a16 + a17) + ((a16 - a17) << bitsPerSum);
		a18 = short(32767 * (pSrc[18] - pRef[18]) + 0.5f);
		a19 = short(32767 * (pSrc[19] - pRef[19]) + 0.5f);
		b9 = (a18 + a19) + ((a18 - a19) << bitsPerSum);
		a20 = short(32767 * (pSrc[20] - pRef[20]) + 0.5f);
		a21 = short(32767 * (pSrc[21] - pRef[21]) + 0.5f);
		b10 = (a20 + a21) + ((a20 - a21) << bitsPerSum);
		a22 = short(32767 * (pSrc[22] - pRef[22]) + 0.5f);
		a23 = short(32767 * (pSrc[23] - pRef[23]) + 0.5f);
		b11 = (a22 + a23) + ((a22 - a23) << bitsPerSum);
		a24 = short(32767 * (pSrc[24] - pRef[24]) + 0.5f);
		a25 = short(32767 * (pSrc[25] - pRef[25]) + 0.5f);
		b12 = (a24 + a25) + ((a24 - a25) << bitsPerSum);
		a26 = short(32767 * (pSrc[26] - pRef[26]) + 0.5f);
		a27 = short(32767 * (pSrc[27] - pRef[27]) + 0.5f);
		b13 = (a26 + a27) + ((a26 - a27) << bitsPerSum);
		a28 = short(32767 * (pSrc[28] - pRef[28]) + 0.5f);
		a29 = short(32767 * (pSrc[29] - pRef[29]) + 0.5f);
		b14 = (a28 + a29) + ((a28 - a29) << bitsPerSum);
		a30 = short(32767 * (pSrc[30] - pRef[30]) + 0.5f);
		a31 = short(32767 * (pSrc[31] - pRef[31]) + 0.5f);
		b15 = (a30 + a31) + ((a30 - a31) << bitsPerSum);
		tmp[i][0] = b0 + b8;
		tmp[i][1] = b0 - b8;
		tmp[i][2] = b1 + b9;
		tmp[i][3] = b1 - b9;
		tmp[i][4] = b2 + b10;
		tmp[i][5] = b2 - b10;
		tmp[i][6] = b3 + b11;
		tmp[i][7] = b3 - b11;
		tmp[i][8] = b4 + b12;
		tmp[i][9] = b4 - b12;
		tmp[i][10] = b5 + b13;
		tmp[i][11] = b5 - b13;
		tmp[i][12] = b6 + b14;
		tmp[i][13] = b6 - b14;
		tmp[i][14] = b7 + b15;
		tmp[i][15] = b7 - b15;

		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}

	for (int i = 0; i < 16; i++) {
		HADAMARD32(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i], tmp[4][i], tmp[5][i], tmp[6][i], tmp[7][i], tmp[8][i], tmp[9][i], tmp[10][i], tmp[11][i], tmp[12][i], tmp[13][i], tmp[14][i], tmp[15][i], tmp[16][i], tmp[17][i], tmp[18][i], tmp[19][i], tmp[20][i], tmp[21][i], tmp[22][i], tmp[23][i], tmp[24][i], tmp[25][i], tmp[26][i], tmp[27][i], tmp[28][i], tmp[29][i], tmp[30][i], tmp[31][i]);
		a0 = abs2<SumType, SumType2>(a0) +abs2<SumType, SumType2>(a1) +abs2<SumType, SumType2>(a2) +abs2<SumType, SumType2>(a3) +abs2<SumType, SumType2>(a4) +abs2<SumType, SumType2>(a5) +abs2<SumType, SumType2>(a6) +abs2<SumType, SumType2>(a7) +abs2<SumType, SumType2>(a8) +abs2<SumType, SumType2>(a9) +abs2<SumType, SumType2>(a10) +abs2<SumType, SumType2>(a11) +abs2<SumType, SumType2>(a12) +abs2<SumType, SumType2>(a13) +abs2<SumType, SumType2>(a14) +abs2<SumType, SumType2>(a15) +abs2<SumType, SumType2>(a16) +abs2<SumType, SumType2>(a17) +abs2<SumType, SumType2>(a18) +abs2<SumType, SumType2>(a19) +abs2<SumType, SumType2>(a20) +abs2<SumType, SumType2>(a21) +abs2<SumType, SumType2>(a22) +abs2<SumType, SumType2>(a23) +abs2<SumType, SumType2>(a24) +abs2<SumType, SumType2>(a25) +abs2<SumType, SumType2>(a26) +abs2<SumType, SumType2>(a27) +abs2<SumType, SumType2>(a28) +abs2<SumType, SumType2>(a29) +abs2<SumType, SumType2>(a30) +abs2<SumType, SumType2>(a31);
		sum += ((SumType)a0) + (a0 >> bitsPerSum);
	}

	return float(double(sum >> 1) / 32767);
}

template <typename PixelType>
float Satd_32x32_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_32x32_C<PixelType, long, long long>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <int nBlkWidth, int nBlkHeight, typename PixelType>
float Satd_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	if (nBlkWidth == 4 && nBlkHeight == 4)
		return Satd_4x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 8 && nBlkHeight == 8)
		return Satd_8x8_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 16 && nBlkHeight == 16)
		return Satd_16x16_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 32 && nBlkHeight == 32)
		return Satd_32x32_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else
		return 0.f;
}

#endif
