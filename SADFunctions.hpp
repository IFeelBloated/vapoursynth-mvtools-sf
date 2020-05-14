#pragma once
#include <cstdint>
#include <cmath>
#include <array>
#include "Include/Interface.hxx"

using SADFunction = auto(*)(const std::uint8_t *, std::intptr_t, const std::uint8_t *, std::intptr_t)->double;

struct dual_double final {
	self(msb, 0.);
	self(lsb, 0.);
	dual_double() = default;
	constexpr dual_double(long double val) {
		lsb = static_cast<decltype(lsb)>(val);
	}
	dual_double(const dual_double &) = default;
	dual_double(dual_double &&) = default;
	auto operator=(const dual_double &)->decltype(*this) = default;
	auto operator=(dual_double &&)->decltype(*this) = default;
	~dual_double() = default;
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
	friend auto operator+(const dual_double &a, const dual_double &b) {
		auto tmp = a;
		tmp += b;
		return tmp;
	}
	friend auto operator-(const dual_double &a, const dual_double &b) {
		auto tmp = a;
		tmp -= b;
		return tmp;
	}
};

constexpr auto operator""_dual(long double val) {
	return dual_double{ val };
}

template<int nBlkWidth, int nBlkHeight>
auto Sad_C(const std::uint8_t *pSrc8, std::intptr_t nSrcPitch, const std::uint8_t *pRef8, std::intptr_t nRefPitch) {
	auto sum = 0.;
	for (auto y = 0; y < nBlkHeight; ++y) {
		for (auto x = 0; x < nBlkWidth; ++x) {
			auto pSrc = reinterpret_cast<const float *>(pSrc8);
			auto pRef = reinterpret_cast<const float *>(pRef8);
			sum += std::abs(static_cast<decltype(sum)>(pSrc[x]) - pRef[x]);
		}
		pSrc8 += nSrcPitch;
		pRef8 += nRefPitch;
	}
	return sum;
}

template<int nBlkWidth, int nBlkHeight>
auto Satd_C(const std::uint8_t *pSrc8, std::intptr_t nSrcPitch, const std::uint8_t *pRef8, std::intptr_t nRefPitch) {
	auto hadamard4 = [](auto &d0, auto &d1, auto &d2, auto &d3, auto &s0, auto &s1, auto &s2, auto &s3) {
		auto t0 = s0 + s1;
		auto t1 = s0 - s1;
		auto t2 = s2 + s3;
		auto t3 = s2 - s3;
		d0 = t0 + t2;
		d2 = t0 - t2;
		d1 = t1 + t3;
		d3 = t1 - t3;
	};
	auto Satd_4x4_C = [&]() {
		using array_t = std::array<decltype(0._dual), 2>;
		auto tmp = std::array<array_t, 4>{};
		auto a0 = 0._dual, a1 = 0._dual, a2 = 0._dual, a3 = 0._dual, b0 = 0._dual, b1 = 0._dual;
		auto sum = 0.;
		for (auto i = 0; i < 4; ++i) {
			auto pSrc = reinterpret_cast<const float *>(pSrc8);
			auto pRef = reinterpret_cast<const float *>(pRef8);
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
			hadamard4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
			a0 = a0.abs() + a1.abs() + a2.abs() + a3.abs();
			sum += a0.lsb + a0.msb;
		}
		return sum / 2.;
	};
	auto Satd_8x4_C = [&](auto pSrc8, auto pRef8) {
		using array_t = std::array<decltype(0._dual), 4>;
		auto tmp = std::array<array_t, 4>{};
		auto a0 = 0._dual, a1 = 0._dual, a2 = 0._dual, a3 = 0._dual;
		auto sum = 0._dual;
		for (auto i = 0; i < 4; ++i) {
			auto pSrc = reinterpret_cast<const float *>(pSrc8);
			auto pRef = reinterpret_cast<const float *>(pRef8);
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
			hadamard4(tmp[i][0], tmp[i][1], tmp[i][2], tmp[i][3], a0, a1, a2, a3);
			pSrc8 += nSrcPitch;
			pRef8 += nRefPitch;
		}
		for (auto i = 0; i < 4; ++i) {
			hadamard4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
			sum += a0.abs() + a1.abs() + a2.abs() + a3.abs();
		}
		return (sum.lsb + sum.msb) / 2.;
	};
	if (nBlkWidth == 4 && nBlkHeight == 4)
		return Satd_4x4_C();
	else {
		constexpr auto bytesPerSample = sizeof(float);
		constexpr auto partition_width = 8;
		constexpr auto partition_height = 4;
		auto sum = 0.;
		for (auto y = 0; y < nBlkHeight; y += partition_height) {
			for (auto x = 0; x < nBlkWidth; x += partition_width)
				sum += Satd_8x4_C(pSrc8 + x * bytesPerSample, pRef8 + x * bytesPerSample);
			pSrc8 += nSrcPitch * partition_height;
			pRef8 += nRefPitch * partition_height;
		}
		return sum;
	}
}
