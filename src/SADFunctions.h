#ifndef __SAD_FUNC__
#define __SAD_FUNC__

#define HADAMARD4(d0, d1, d2, d3, s0, s1, s2, s3) {\
    SumType t0 = s0 + s1;\
    SumType t1 = s0 - s1;\
    SumType t2 = s2 + s3;\
    SumType t3 = s2 - s3;\
    d0 = t0 + t2;\
    d2 = t0 - t2;\
    d1 = t1 + t3;\
    d3 = t1 - t3;\
}

#define HADAMARD8(d0, d1, d2, d3, d4, d5, d6, d7, s0, s1, s2, s3, s4, s5, s6, s7) {\
    SumType t0 = s0 + s1;\
    SumType t1 = s0 - s1;\
    SumType t2 = s2 + s3;\
    SumType t3 = s2 - s3;\
    SumType t4 = s4 + s5;\
    SumType t5 = s4 - s5;\
    SumType t6 = s6 + s7;\
    SumType t7 = s6 - s7;\
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
    SumType t0 = s0 + s1;\
    SumType t1 = s0 - s1;\
    SumType t2 = s2 + s3;\
    SumType t3 = s2 - s3;\
    SumType t4 = s4 + s5;\
    SumType t5 = s4 - s5;\
    SumType t6 = s6 + s7;\
    SumType t7 = s6 - s7;\
    SumType t8 = s8 + s9;\
    SumType t9 = s8 - s9;\
    SumType t10 = s10 + s11;\
    SumType t11 = s10 - s11;\
    SumType t12 = s12 + s13;\
    SumType t13 = s12 - s13;\
    SumType t14 = s14 + s15;\
    SumType t15 = s14 - s15;\
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
    SumType t0 = s0 + s1;\
    SumType t1 = s0 - s1;\
    SumType t2 = s2 + s3;\
    SumType t3 = s2 - s3;\
    SumType t4 = s4 + s5;\
    SumType t5 = s4 - s5;\
    SumType t6 = s6 + s7;\
    SumType t7 = s6 - s7;\
    SumType t8 = s8 + s9;\
    SumType t9 = s8 - s9;\
    SumType t10 = s10 + s11;\
    SumType t11 = s10 - s11;\
    SumType t12 = s12 + s13;\
    SumType t13 = s12 - s13;\
    SumType t14 = s14 + s15;\
    SumType t15 = s14 - s15;\
    SumType t16 = s16 + s17;\
    SumType t17 = s16 - s17;\
    SumType t18 = s18 + s19;\
    SumType t19 = s18 - s19;\
    SumType t20 = s20 + s21;\
    SumType t21 = s20 - s21;\
    SumType t22 = s22 + s23;\
    SumType t23 = s22 - s23;\
    SumType t24 = s24 + s25;\
    SumType t25 = s24 - s25;\
    SumType t26 = s26 + s27;\
    SumType t27 = s26 - s27;\
    SumType t28 = s28 + s29;\
    SumType t29 = s28 - s29;\
    SumType t30 = s30 + s31;\
    SumType t31 = s30 - s31;\
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

#include <cstdint>

typedef double(*SADFunction)(const uint8_t *pSrc, intptr_t nSrcPitch,
	const uint8_t *pRef, intptr_t nRefPitch);

template<typename PixelType>
static inline PixelType SADABS(PixelType x) {
	return (x < 0) ? -x : x;
}

template<int32_t nBlkWidth, int32_t nBlkHeight, typename PixelType>
double Sad_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	double sum = 0.;
	for (int32_t y = 0; y < nBlkHeight; y++) {
		for (int32_t x = 0; x < nBlkWidth; x++) {
			const PixelType *pSrc = (const PixelType *)pSrc8;
			const PixelType *pRef = (const PixelType *)pRef8;
			sum += SADABS<double>(static_cast<double>(pSrc[x]) - pRef[x]);
		}
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	return sum;
}

template <typename PixelType, typename SumType>
double Real_Satd_4x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	SumType tmp[4][2][2];
	SumType a0, a1, a2, a3, b0a, b1a, b0b, b1b;
	SumType sum = 0.;
	for (int32_t i = 0; i < 4; ++i) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;
		a0 = static_cast<SumType>(pSrc[0]) - pRef[0];
		a1 = static_cast<SumType>(pSrc[1]) - pRef[1];
		b0a = a0 + a1;
		b0b = a0 - a1;
		a2 = static_cast<SumType>(pSrc[2]) - pRef[2];
		a3 = static_cast<SumType>(pSrc[3]) - pRef[3];
		b1a = a2 + a3;
		b1b = a2 - a3;
		tmp[i][0][0] = b0a + b1a;
		tmp[i][1][0] = b0a - b1a;
		tmp[i][0][1] = b0b + b1b;
		tmp[i][1][1] = b0b - b1b;
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (int32_t i = 0; i < 2; ++i) {
		HADAMARD4(a0, a1, a2, a3, tmp[0][i][0], tmp[1][i][0], tmp[2][i][0], tmp[3][i][0]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3);
		sum += a0;
		HADAMARD4(a0, a1, a2, a3, tmp[0][i][1], tmp[1][i][1], tmp[2][i][1], tmp[3][i][1]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3);
		sum += a0;
	}
	return sum / 2.;
}

template <typename PixelType>
double Satd_4x4_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_4x4_C<PixelType, double>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType>
double Real_Satd_8x8_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	SumType tmp[8][4][2];
	SumType a0, a1, a2, a3, a4, a5, a6, a7, b0a, b1a, b2a, b3a, b0b, b1b, b2b, b3b;
	SumType sum = 0.;
	for (int32_t i = 0; i < 8; ++i) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;
		a0 = static_cast<SumType>(pSrc[0]) - pRef[0];
		a1 = static_cast<SumType>(pSrc[1]) - pRef[1];
		b0a = a0 + a1;
		b0b = a0 - a1;
		a2 = static_cast<SumType>(pSrc[2]) - pRef[2];
		a3 = static_cast<SumType>(pSrc[3]) - pRef[3];
		b1a = a2 + a3;
		b1b = a2 - a3;
		a4 = static_cast<SumType>(pSrc[4]) - pRef[4];
		a5 = static_cast<SumType>(pSrc[5]) - pRef[5];
		b2a = a4 + a5;
		b2b = a4 - a5;
		a6 = static_cast<SumType>(pSrc[6]) - pRef[6];
		a7 = static_cast<SumType>(pSrc[7]) - pRef[7];
		b3a = a6 + a7;
		b3b = a6 - a7;
		tmp[i][0][0] = b0a + b2a;
		tmp[i][1][0] = b0a - b2a;
		tmp[i][2][0] = b1a + b3a;
		tmp[i][3][0] = b1a - b3a;
		tmp[i][0][1] = b0b + b2b;
		tmp[i][1][1] = b0b - b2b;
		tmp[i][2][1] = b1b + b3b;
		tmp[i][3][1] = b1b - b3b;
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (int32_t i = 0; i < 4; ++i) {
		HADAMARD8(a0, a1, a2, a3, a4, a5, a6, a7, tmp[0][i][0], tmp[1][i][0], tmp[2][i][0], tmp[3][i][0], tmp[4][i][0], tmp[5][i][0], tmp[6][i][0], tmp[7][i][0]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3) + SADABS<SumType>(a4) + SADABS<SumType>(a5) + SADABS<SumType>(a6) + SADABS<SumType>(a7);
		sum += a0;
		HADAMARD8(a0, a1, a2, a3, a4, a5, a6, a7, tmp[0][i][1], tmp[1][i][1], tmp[2][i][1], tmp[3][i][1], tmp[4][i][1], tmp[5][i][1], tmp[6][i][1], tmp[7][i][1]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3) + SADABS<SumType>(a4) + SADABS<SumType>(a5) + SADABS<SumType>(a6) + SADABS<SumType>(a7);
		sum += a0;
	}
	return sum / 2.;
}

template <typename PixelType>
double Satd_8x8_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_8x8_C<PixelType, double>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType>
double Real_Satd_16x16_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	SumType tmp[16][8][2];
	SumType a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, b0a, b1a, b2a, b3a, b4a, b5a, b6a, b7a, b0b, b1b, b2b, b3b, b4b, b5b, b6b, b7b;
	SumType sum = 0.;
	for (int32_t i = 0; i < 16; ++i) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;
		a0 = static_cast<SumType>(pSrc[0]) - pRef[0];
		a1 = static_cast<SumType>(pSrc[1]) - pRef[1];
		b0a = a0 + a1;
		b0b = a0 - a1;
		a2 = static_cast<SumType>(pSrc[2]) - pRef[2];
		a3 = static_cast<SumType>(pSrc[3]) - pRef[3];
		b1a = a2 + a3;
		b1b = a2 - a3;
		a4 = static_cast<SumType>(pSrc[4]) - pRef[4];
		a5 = static_cast<SumType>(pSrc[5]) - pRef[5];
		b2a = a4 + a5;
		b2b = a4 - a5;
		a6 = static_cast<SumType>(pSrc[6]) - pRef[6];
		a7 = static_cast<SumType>(pSrc[7]) - pRef[7];
		b3a = a6 + a7;
		b3b = a6 - a7;
		a8 = static_cast<SumType>(pSrc[8]) - pRef[8];
		a9 = static_cast<SumType>(pSrc[9]) - pRef[9];
		b4a = a8 + a9;
		b4b = a8 - a9;
		a10 = static_cast<SumType>(pSrc[10]) - pRef[10];
		a11 = static_cast<SumType>(pSrc[11]) - pRef[11];
		b5a = a10 + a11;
		b5b = a10 - a11;
		a12 = static_cast<SumType>(pSrc[12]) - pRef[12];
		a13 = static_cast<SumType>(pSrc[13]) - pRef[13];
		b6a = a12 + a13;
		b6b = a12 - a13;
		a14 = static_cast<SumType>(pSrc[14]) - pRef[14];
		a15 = static_cast<SumType>(pSrc[15]) - pRef[15];
		b7a = a14 + a15;
		b7b = a14 - a15;
		tmp[i][0][0] = b0a + b4a;
		tmp[i][1][0] = b0a - b4a;
		tmp[i][2][0] = b1a + b5a;
		tmp[i][3][0] = b1a - b5a;
		tmp[i][4][0] = b2a + b6a;
		tmp[i][5][0] = b2a - b6a;
		tmp[i][6][0] = b3a + b7a;
		tmp[i][7][0] = b3a - b7a;
		tmp[i][0][1] = b0b + b4b;
		tmp[i][1][1] = b0b - b4b;
		tmp[i][2][1] = b1b + b5b;
		tmp[i][3][1] = b1b - b5b;
		tmp[i][4][1] = b2b + b6b;
		tmp[i][5][1] = b2b - b6b;
		tmp[i][6][1] = b3b + b7b;
		tmp[i][7][1] = b3b - b7b;
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (int32_t i = 0; i < 8; ++i) {
		HADAMARD16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, tmp[0][i][0], tmp[1][i][0], tmp[2][i][0], tmp[3][i][0], tmp[4][i][0], tmp[5][i][0], tmp[6][i][0], tmp[7][i][0], tmp[8][i][0], tmp[9][i][0], tmp[10][i][0], tmp[11][i][0], tmp[12][i][0], tmp[13][i][0], tmp[14][i][0], tmp[15][i][0]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3) + SADABS<SumType>(a4) + SADABS<SumType>(a5) + SADABS<SumType>(a6) + SADABS<SumType>(a7) + SADABS<SumType>(a8) + SADABS<SumType>(a9) + SADABS<SumType>(a10) + SADABS<SumType>(a11) + SADABS<SumType>(a12) + SADABS<SumType>(a13) + SADABS<SumType>(a14) + SADABS<SumType>(a15);
		sum += a0;
		HADAMARD16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, tmp[0][i][1], tmp[1][i][1], tmp[2][i][1], tmp[3][i][1], tmp[4][i][1], tmp[5][i][1], tmp[6][i][1], tmp[7][i][1], tmp[8][i][1], tmp[9][i][1], tmp[10][i][1], tmp[11][i][1], tmp[12][i][1], tmp[13][i][1], tmp[14][i][1], tmp[15][i][1]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3) + SADABS<SumType>(a4) + SADABS<SumType>(a5) + SADABS<SumType>(a6) + SADABS<SumType>(a7) + SADABS<SumType>(a8) + SADABS<SumType>(a9) + SADABS<SumType>(a10) + SADABS<SumType>(a11) + SADABS<SumType>(a12) + SADABS<SumType>(a13) + SADABS<SumType>(a14) + SADABS<SumType>(a15);
		sum += a0;
	}
	return sum / 2.;
}

template <typename PixelType>
double Satd_16x16_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_16x16_C<PixelType, double>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <typename PixelType, typename SumType>
double Real_Satd_32x32_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8, intptr_t nRefPitch) {
	SumType tmp[32][16][2];
	SumType a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, b0a, b1a, b2a, b3a, b4a, b5a, b6a, b7a, b8a, b9a, b10a, b11a, b12a, b13a, b14a, b15a, b0b, b1b, b2b, b3b, b4b, b5b, b6b, b7b, b8b, b9b, b10b, b11b, b12b, b13b, b14b, b15b;
	SumType sum = 0.;
	for (int32_t i = 0; i < 32; ++i) {
		const PixelType *pSrc = (const PixelType *)pSrc8;
		const PixelType *pRef = (const PixelType *)pRef8;
		a0 = static_cast<SumType>(pSrc[0]) - pRef[0];
		a1 = static_cast<SumType>(pSrc[1]) - pRef[1];
		b0a = a0 + a1;
		b0b = a0 - a1;
		a2 = static_cast<SumType>(pSrc[2]) - pRef[2];
		a3 = static_cast<SumType>(pSrc[3]) - pRef[3];
		b1a = a2 + a3;
		b1b = a2 - a3;
		a4 = static_cast<SumType>(pSrc[4]) - pRef[4];
		a5 = static_cast<SumType>(pSrc[5]) - pRef[5];
		b2a = a4 + a5;
		b2b = a4 - a5;
		a6 = static_cast<SumType>(pSrc[6]) - pRef[6];
		a7 = static_cast<SumType>(pSrc[7]) - pRef[7];
		b3a = a6 + a7;
		b3b = a6 - a7;
		a8 = static_cast<SumType>(pSrc[8]) - pRef[8];
		a9 = static_cast<SumType>(pSrc[9]) - pRef[9];
		b4a = a8 + a9;
		b4b = a8 - a9;
		a10 = static_cast<SumType>(pSrc[10]) - pRef[10];
		a11 = static_cast<SumType>(pSrc[11]) - pRef[11];
		b5a = a10 + a11;
		b5b = a10 - a11;
		a12 = static_cast<SumType>(pSrc[12]) - pRef[12];
		a13 = static_cast<SumType>(pSrc[13]) - pRef[13];
		b6a = a12 + a13;
		b6b = a12 - a13;
		a14 = static_cast<SumType>(pSrc[14]) - pRef[14];
		a15 = static_cast<SumType>(pSrc[15]) - pRef[15];
		b7a = a14 + a15;
		b7b = a14 - a15;
		a16 = static_cast<SumType>(pSrc[16]) - pRef[16];
		a17 = static_cast<SumType>(pSrc[17]) - pRef[17];
		b8a = a16 + a17;
		b8b = a16 - a17;
		a18 = static_cast<SumType>(pSrc[18]) - pRef[18];
		a19 = static_cast<SumType>(pSrc[19]) - pRef[19];
		b9a = a18 + a19;
		b9b = a18 - a19;
		a20 = static_cast<SumType>(pSrc[20]) - pRef[20];
		a21 = static_cast<SumType>(pSrc[21]) - pRef[21];
		b10a = a20 + a21;
		b10b = a20 - a21;
		a22 = static_cast<SumType>(pSrc[22]) - pRef[22];
		a23 = static_cast<SumType>(pSrc[23]) - pRef[23];
		b11a = a22 + a23;
		b11b = a22 - a23;
		a24 = static_cast<SumType>(pSrc[24]) - pRef[24];
		a25 = static_cast<SumType>(pSrc[25]) - pRef[25];
		b12a = a24 + a25;
		b12b = a24 - a25;
		a26 = static_cast<SumType>(pSrc[26]) - pRef[26];
		a27 = static_cast<SumType>(pSrc[27]) - pRef[27];
		b13a = a26 + a27;
		b13b = a26 - a27;
		a28 = static_cast<SumType>(pSrc[28]) - pRef[28];
		a29 = static_cast<SumType>(pSrc[29]) - pRef[29];
		b14a = a28 + a29;
		b14b = a28 - a29;
		a30 = static_cast<SumType>(pSrc[30]) - pRef[30];
		a31 = static_cast<SumType>(pSrc[31]) - pRef[31];
		b15a = a30 + a31;
		b15b = a30 - a31;
		tmp[i][0][0] = b0a + b8a;
		tmp[i][1][0] = b0a - b8a;
		tmp[i][2][0] = b1a + b9a;
		tmp[i][3][0] = b1a - b9a;
		tmp[i][4][0] = b2a + b10a;
		tmp[i][5][0] = b2a - b10a;
		tmp[i][6][0] = b3a + b11a;
		tmp[i][7][0] = b3a - b11a;
		tmp[i][8][0] = b4a + b12a;
		tmp[i][9][0] = b4a - b12a;
		tmp[i][10][0] = b5a + b13a;
		tmp[i][11][0] = b5a - b13a;
		tmp[i][12][0] = b6a + b14a;
		tmp[i][13][0] = b6a - b14a;
		tmp[i][14][0] = b7a + b15a;
		tmp[i][15][0] = b7a - b15a;
		tmp[i][0][1] = b0b + b8b;
		tmp[i][1][1] = b0b - b8b;
		tmp[i][2][1] = b1b + b9b;
		tmp[i][3][1] = b1b - b9b;
		tmp[i][4][1] = b2b + b10b;
		tmp[i][5][1] = b2b - b10b;
		tmp[i][6][1] = b3b + b11b;
		tmp[i][7][1] = b3b - b11b;
		tmp[i][8][1] = b4b + b12b;
		tmp[i][9][1] = b4b - b12b;
		tmp[i][10][1] = b5b + b13b;
		tmp[i][11][1] = b5b - b13b;
		tmp[i][12][1] = b6b + b14b;
		tmp[i][13][1] = b6b - b14b;
		tmp[i][14][1] = b7b + b15b;
		tmp[i][15][1] = b7b - b15b;
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (int32_t i = 0; i < 16; ++i) {
		HADAMARD32(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, tmp[0][i][0], tmp[1][i][0], tmp[2][i][0], tmp[3][i][0], tmp[4][i][0], tmp[5][i][0], tmp[6][i][0], tmp[7][i][0], tmp[8][i][0], tmp[9][i][0], tmp[10][i][0], tmp[11][i][0], tmp[12][i][0], tmp[13][i][0], tmp[14][i][0], tmp[15][i][0], tmp[16][i][0], tmp[17][i][0], tmp[18][i][0], tmp[19][i][0], tmp[20][i][0], tmp[21][i][0], tmp[22][i][0], tmp[23][i][0], tmp[24][i][0], tmp[25][i][0], tmp[26][i][0], tmp[27][i][0], tmp[28][i][0], tmp[29][i][0], tmp[30][i][0], tmp[31][i][0]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3) + SADABS<SumType>(a4) + SADABS<SumType>(a5) + SADABS<SumType>(a6) + SADABS<SumType>(a7) + SADABS<SumType>(a8) + SADABS<SumType>(a9) + SADABS<SumType>(a10) + SADABS<SumType>(a11) + SADABS<SumType>(a12) + SADABS<SumType>(a13) + SADABS<SumType>(a14) + SADABS<SumType>(a15) + SADABS<SumType>(a16) + SADABS<SumType>(a17) + SADABS<SumType>(a18) + SADABS<SumType>(a19) + SADABS<SumType>(a20) + SADABS<SumType>(a21) + SADABS<SumType>(a22) + SADABS<SumType>(a23) + SADABS<SumType>(a24) + SADABS<SumType>(a25) + SADABS<SumType>(a26) + SADABS<SumType>(a27) + SADABS<SumType>(a28) + SADABS<SumType>(a29) + SADABS<SumType>(a30) + SADABS<SumType>(a31);
		sum += a0;
		HADAMARD32(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, tmp[0][i][1], tmp[1][i][1], tmp[2][i][1], tmp[3][i][1], tmp[4][i][1], tmp[5][i][1], tmp[6][i][1], tmp[7][i][1], tmp[8][i][1], tmp[9][i][1], tmp[10][i][1], tmp[11][i][1], tmp[12][i][1], tmp[13][i][1], tmp[14][i][1], tmp[15][i][1], tmp[16][i][1], tmp[17][i][1], tmp[18][i][1], tmp[19][i][1], tmp[20][i][1], tmp[21][i][1], tmp[22][i][1], tmp[23][i][1], tmp[24][i][1], tmp[25][i][1], tmp[26][i][1], tmp[27][i][1], tmp[28][i][1], tmp[29][i][1], tmp[30][i][1], tmp[31][i][1]);
		a0 = SADABS<SumType>(a0) + SADABS<SumType>(a1) + SADABS<SumType>(a2) + SADABS<SumType>(a3) + SADABS<SumType>(a4) + SADABS<SumType>(a5) + SADABS<SumType>(a6) + SADABS<SumType>(a7) + SADABS<SumType>(a8) + SADABS<SumType>(a9) + SADABS<SumType>(a10) + SADABS<SumType>(a11) + SADABS<SumType>(a12) + SADABS<SumType>(a13) + SADABS<SumType>(a14) + SADABS<SumType>(a15) + SADABS<SumType>(a16) + SADABS<SumType>(a17) + SADABS<SumType>(a18) + SADABS<SumType>(a19) + SADABS<SumType>(a20) + SADABS<SumType>(a21) + SADABS<SumType>(a22) + SADABS<SumType>(a23) + SADABS<SumType>(a24) + SADABS<SumType>(a25) + SADABS<SumType>(a26) + SADABS<SumType>(a27) + SADABS<SumType>(a28) + SADABS<SumType>(a29) + SADABS<SumType>(a30) + SADABS<SumType>(a31);
		sum += a0;
	}
	return sum / 2.;
}

template <typename PixelType>
double Satd_32x32_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	return Real_Satd_32x32_C<PixelType, double>(pSrc, nSrcPitch, pRef, nRefPitch);
}

template <int32_t nBlkWidth, int32_t nBlkHeight, typename PixelType>
double Satd_C(const uint8_t *pSrc, intptr_t nSrcPitch, const uint8_t *pRef, intptr_t nRefPitch) {
	if (nBlkWidth == 4 && nBlkHeight == 4)
		return Satd_4x4_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 8 && nBlkHeight == 8)
		return Satd_8x8_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 16 && nBlkHeight == 16)
		return Satd_16x16_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else if (nBlkWidth == 32 && nBlkHeight == 32)
		return Satd_32x32_C<PixelType>(pSrc, nSrcPitch, pRef, nRefPitch);
	else
		return 0.;
}

#endif
