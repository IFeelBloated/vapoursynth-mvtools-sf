#pragma once
#include <cstdint>
#include <cmath>
#include "Cosmetics.hpp"

using SADFunction = auto(*)(const uint8_t *, intptr_t, const uint8_t *, intptr_t)->double;

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

static auto StoreAsInteger = [](auto value) {
	return reinterpret_cast<std::int32_t &>(value);
};

static auto CastBackToFloat = [](auto value) {
	return reinterpret_cast<float &>(value);
};

struct dual_double final {
	self(msb, 0.);
	self(lsb, 0.);
	auto &lsb2msb() {
		msb = lsb;
		return *this;
	}
	auto &msb2lsb() {
		lsb = msb;
		return *this;
	}
	auto &abs() {
		msb = std::abs(msb);
		lsb = std::abs(lsb);
		return *this;
	}
	dual_double() = default;
	constexpr dual_double(long double val) {
		lsb = static_cast<decltype(lsb)>(val);
	}
	dual_double(const dual_double &) = default;
	dual_double(dual_double &&) = default;
	auto operator=(const dual_double &)->decltype(*this) = default;
	auto operator=(dual_double &&)->decltype(*this) = default;
	~dual_double() = default;
	auto &operator+=(const dual_double &val) {
		msb += val.msb;
		lsb += val.lsb;
		return *this;
	}
	auto &operator-=(const dual_double &val) {
		msb -= val.msb;
		lsb -= val.lsb;
		return *this;
	}
};

constexpr auto operator""_dual(long double val) {
	return dual_double{ val };
}

static inline auto operator+(const dual_double &a, const dual_double &b) {
	auto tmp = a;
	tmp += b;
	return tmp;
}

static inline auto operator-(const dual_double &a, const dual_double &b) {
	auto tmp = a;
	tmp -= b;
	return tmp;
}

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

template <typename PixelType>
auto Satd_4x4_C(const uint8_t *pSrc8, intptr_t nSrcPitch, const uint8_t *pRef8,
	intptr_t nRefPitch) {
	decltype(0._dual) tmp[4][2];
	auto a0 = 0._dual, a1 = 0._dual, a2 = 0._dual, a3 = 0._dual, b0 = 0._dual, b1 = 0._dual;
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
	decltype(0._dual) tmp[4][4];
	auto a0 = 0._dual, a1 = 0._dual, a2 = 0._dual, a3 = 0._dual;
	auto sum = 0._dual;
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
}
