#ifndef __SAD_FUNC__
#define __SAD_FUNC__
#include <cstdint>
#include <cmath>
<<<<<<< HEAD

struct dual_double final {
	double msb = 0.;
	double lsb = 0.;
	auto lsb2msb()->dual_double & {
		msb = lsb;
		return *this;
	}
	auto msb2lsb()->dual_double & {
		lsb = msb;
		return *this;
	}
	auto abs()->dual_double & {
		msb = std::abs(msb);
		lsb = std::abs(lsb);
		return *this;
	}
	dual_double() = default;
	dual_double(double val) {
		lsb = val;
	}
	dual_double(const dual_double &) = default;
	auto operator=(const dual_double &)->dual_double & = default;
	~dual_double() = default;
	auto operator+=(const dual_double &val) {
		msb += val.msb;
		lsb += val.lsb;
	}
};

static auto operator+(const dual_double &a, const dual_double &b) {
	auto tmp = a;
	tmp.msb += b.msb;
	tmp.lsb += b.lsb;
	return tmp;
}

static auto operator-(const dual_double &a, const dual_double &b) {
	auto tmp = a;
	tmp.msb -= b.msb;
	tmp.lsb -= b.lsb;
	return tmp;
=======
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
>>>>>>> origin/master
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

<<<<<<< HEAD
#define HADAMARD4(d0, d1, d2, d3, s0, s1, s2, s3) { \
	auto t0 = s0 + s1;                              \
	auto t1 = s0 - s1;                              \
	auto t2 = s2 + s3;                              \
	auto t3 = s2 - s3;                              \
	d0 = t0 + t2;                                   \
	d2 = t0 - t2;                                   \
	d1 = t1 + t3;                                   \
	d3 = t1 - t3;                                   \
}

template <typename PixelType>
auto Satd_4x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	dual_double tmp[4][2];
	dual_double a0, a1, a2, a3, b0, b1;
	auto sum = 0.;
	for (auto i = 0; i < 4; ++i) {
		auto pSrc = reinterpret_cast<const PixelType *>(pSrc8);
		auto pRef = reinterpret_cast<const PixelType *>(pRef8);
		a0 = static_cast<double>(pSrc[0]) - pRef[0];
		a1 = static_cast<double>(pSrc[1]) - pRef[1];
		b0 = (a0 + a1) + (a0 - a1).lsb2msb();
		a2 = static_cast<double>(pSrc[2]) - pRef[2];
		a3 = static_cast<double>(pSrc[3]) - pRef[3];
		b1 = (a2 + a3) + (a2 - a3).lsb2msb();
		tmp[i][0] = b0 + b1;
		tmp[i][1] = b0 - b1;
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (auto i = 0; i < 2; ++i) {
		HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
		a0 = a0.abs() + a1.abs() + a2.abs() + a3.abs();
		sum += a0.lsb + a0.msb;
	}
	return sum / 2.;
}

template <typename PixelType>
auto Satd_8x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	dual_double tmp[4][4];
	dual_double a0, a1, a2, a3;
	dual_double sum;
	for (auto i = 0; i < 4; ++i) {
		auto pSrc = reinterpret_cast<const PixelType *>(pSrc8);
		auto pRef = reinterpret_cast<const PixelType *>(pRef8);
		a0 = static_cast<double>(pSrc[4]) - pRef[4];
		a0.lsb2msb();
		a0 += static_cast<double>(pSrc[0]) - pRef[0];
		a1 = static_cast<double>(pSrc[5]) - pRef[5];
		a1.lsb2msb();
		a1 += static_cast<double>(pSrc[1]) - pRef[1];
		a2 = static_cast<double>(pSrc[6]) - pRef[6];
		a2.lsb2msb();
		a2 += static_cast<double>(pSrc[2]) - pRef[2];
		a3 = static_cast<double>(pSrc[7]) - pRef[7];
		a3.lsb2msb();
		a3 += static_cast<double>(pSrc[3]) - pRef[3];
		HADAMARD4(tmp[i][0], tmp[i][1], tmp[i][2], tmp[i][3], a0, a1, a2, a3);
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	for (auto i = 0; i < 4; ++i) {
		HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
		sum += a0.abs() + a1.abs() + a2.abs() + a3.abs();
	}
	return (sum.lsb + sum.msb) / 2.;
}

template<int nBlkWidth, int nBlkHeight, typename PixelType>
auto Satd_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	if (nBlkWidth == 4 && nBlkHeight == 4)
		return Satd_4x4_C<PixelType>(pSrc8, nSrcPitch, pRef8, nRefPitch);
	else {
		constexpr auto bytesPerSample = sizeof(PixelType);
		constexpr auto partition_width = 8;
		constexpr auto partition_height = 4;
		auto sum = 0.;
		for (auto y = 0; y < nBlkHeight; y += partition_height) {
			for (auto x = 0; x < nBlkWidth; x += partition_width)
				sum += Satd_8x4_C<PixelType>(pSrc8 + x * bytesPerSample, nSrcPitch, pRef8 + x * bytesPerSample, nRefPitch);
			pSrc8 += nSrcPitch * partition_height;
			pRef8 += nRefPitch * partition_height;
		}
		return sum;
	}
=======
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
>>>>>>> origin/master
}

#endif
