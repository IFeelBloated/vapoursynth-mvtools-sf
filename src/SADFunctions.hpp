#ifndef __SAD_FUNC__
#define __SAD_FUNC__
#include <cstdint>
#include <cmath>
#include <cstring>

static auto uninitialized = true;
static constexpr auto init_val = 1.;
static decltype(init_val + 0) hadamard_matrix_2x2[2][2];
static decltype(init_val + 0) hadamard_matrix_4x4[4][4];
static decltype(init_val + 0) hadamard_matrix_8x8[8][8];
static decltype(init_val + 0) hadamard_matrix_16x16[16][16];
static decltype(init_val + 0) hadamard_matrix_32x32[32][32];

template<int length = 2, typename T = double>
auto create_Hadamard_matrix(void *dst, const void *src) {
	constexpr auto src_length = length >> 1;
	const auto coeff = std::sqrt(2.);
	auto actual_dst = reinterpret_cast<T(*)[length]>(dst);
	auto actual_src = reinterpret_cast<const T(*)[src_length]>(src);
	for (auto i = 0; i < src_length; ++i)
		std::memcpy(actual_dst[i], actual_src[i], sizeof(actual_src[0]));
	for (auto i = 0; i < src_length; ++i)
		std::memcpy(actual_dst[src_length + i], actual_src[i], sizeof(actual_src[0]));
	auto ptr = reinterpret_cast<T(*)[2][src_length]>(dst);
	for (auto i = 0; i <length; ++i)
		std::memcpy(ptr[i][1], ptr[i][0], sizeof(actual_src[0]));
	ptr += src_length;
	for (auto i = 0; i < src_length; ++i)
		for (auto &x : ptr[i][1])
			x = -x;
	for (auto i = 0; i < length; ++i)
		for (auto &x : actual_dst[i])
			x /= coeff;
}

static auto SATD_init() {
	create_Hadamard_matrix(hadamard_matrix_2x2, &init_val);
	create_Hadamard_matrix<4>(hadamard_matrix_4x4, hadamard_matrix_2x2);
	create_Hadamard_matrix<8>(hadamard_matrix_8x8, hadamard_matrix_4x4);
	create_Hadamard_matrix<16>(hadamard_matrix_16x16, hadamard_matrix_8x8);
	create_Hadamard_matrix<32>(hadamard_matrix_32x32, hadamard_matrix_16x16);
	uninitialized = false;
}

template<int length = 2, typename T = double>
auto product_calc(const void *src, const void *hadamard, void *dst) {
	auto actual_hadamard = reinterpret_cast<const T(*)[length]>(hadamard);
	auto actual_src = reinterpret_cast<const T(*)[length]>(src);
	auto actual_dst = reinterpret_cast<T(*)[length]>(dst);
	auto dot_p = [&](auto row, auto column) {
		T sum = 0;
		for (auto i = 0; i < length; ++i)
			sum += actual_hadamard[row][i] * actual_src[i][column];
		return sum;
	};
	for (auto i = 0; i < length; ++i)
		for (auto j = 0; j < length; ++j)
			actual_dst[i][j] = dot_p(i, j);
}

static inline auto _fakeint(float a) {
	return reinterpret_cast<int32_t &>(a);
}

static inline auto _back2flt(int32_t a) {
	return reinterpret_cast<float &>(a);
}

typedef auto(*SADFunction)(const uint8_t *pSrc, intptr_t nSrcPitch,
	const uint8_t *pRef, intptr_t nRefPitch)->double;

template<int nBlkWidth, int nBlkHeight, typename PixelType>
auto Sad_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	auto sum = 0.;
	for (auto y = 0; y < nBlkHeight; ++y) {
		for (auto x = 0; x < nBlkWidth; ++x) {
			auto pSrc = reinterpret_cast<const PixelType *>(pSrc8);
			auto pRef = reinterpret_cast<const PixelType *>(pRef8);
			sum += std::abs(static_cast<decltype(sum)>(pSrc[x]) - pRef[x]);
		}
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	return sum;
}

template<int nBlkWidth, int nBlkHeight, typename PixelType>
auto Satd_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	if (uninitialized)
		SATD_init();
	void *hadamard;
	if (nBlkWidth == 32 && nBlkHeight == 32)
		hadamard = hadamard_matrix_32x32;
	else if (nBlkWidth == 16 && nBlkHeight == 16)
		hadamard = hadamard_matrix_16x16;
	else if (nBlkWidth == 8 && nBlkHeight == 8)
		hadamard = hadamard_matrix_8x8;
	else if (nBlkWidth == 4 && nBlkHeight == 4)
		hadamard = hadamard_matrix_4x4;
	else
		hadamard = nullptr;
	auto sum = 0.;
	decltype(sum) _dif_block[nBlkHeight][nBlkWidth];
	decltype(sum) _transformed_block[nBlkHeight][nBlkWidth];
	for (auto y = 0; y < nBlkHeight; ++y) {
		for (auto x = 0; x < nBlkWidth; ++x) {
			auto pSrc = reinterpret_cast<const PixelType *>(pSrc8);
			auto pRef = reinterpret_cast<const PixelType *>(pRef8);
			_dif_block[y][x] = static_cast<decltype(sum)>(pSrc[x]) - pRef[x];
		}
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	product_calc<nBlkWidth>(_dif_block, hadamard, _transformed_block);
	for (auto &x : _transformed_block)
		for (auto y : x)
			sum += std::abs(y);
	return sum;
}

#endif
