#pragma once
#include <algorithm>
#include <cmath>
#include <mutex>
#include "Include/fftw3.h"
#include "DCT.hpp"
#include "Include/Interface.hxx"

static auto &&g_fftw_plans_mutex = std::mutex{};

class DCTFFTW final :public DCTClass {
	self(fSrc, static_cast<double *>(nullptr));
	self(dctplan, static_cast<fftw_plan>(nullptr));
	self(fSrcDCT, static_cast<double *>(nullptr));
	self(dctshift, 0);
	self(dctshift0, 0);
public:
	DCTFFTW() = delete;
	DCTFFTW(int _sizex, int _sizey, int _dctmode) {
		sizex = _sizex;
		sizey = _sizey;
		dctmode = _dctmode;
		auto size2d = sizey * sizex;
		auto cursize = 1;
		while (cursize < size2d) {
			++dctshift;
			cursize <<= 1;
		}
		dctshift0 = dctshift + 2;
		fSrc = reinterpret_cast<double *>(fftw_malloc(sizeof(double) * size2d));
		fSrcDCT = reinterpret_cast<double *>(fftw_malloc(sizeof(double) * size2d));
		g_fftw_plans_mutex.lock();
		dctplan = fftw_plan_r2r_2d(sizey, sizex, fSrc, fSrcDCT, FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE);
		g_fftw_plans_mutex.unlock();
	}
	DCTFFTW(DCTFFTW &&) = delete;
	DCTFFTW(const DCTFFTW &&) = delete;
	auto &operator=(DCTFFTW &&) = delete;
	auto &operator=(const DCTFFTW &) = delete;
	~DCTFFTW() override {
		fftw_destroy_plan(dctplan);
		fftw_free(fSrc);
		fftw_free(fSrcDCT);
	}
	auto DCTBytes2D(const std::uint8_t *srcp, int src_pitch, std::uint8_t *dctp, int dct_pitch)->void override {
		auto Bytes2Float = [&]() {
			auto floatpitch = sizex;
			auto realdata = fSrc;
			for (auto j = 0; j < sizey; ++j) {
				for (auto i = 0; i < sizex; ++i) {
					auto real_srcp = reinterpret_cast<const float *>(srcp);
					realdata[i] = real_srcp[i];
				}
				srcp += src_pitch;
				realdata += floatpitch;
			}
		};
		auto Float2Bytes = [&]() {
			constexpr auto coeff = 0.70710678118654752440084436210485;
			auto dstp = reinterpret_cast<float *>(dctp);
			auto realdata = fSrcDCT;
			dct_pitch /= sizeof(float);
			auto floatpitch = sizex;
			auto f = realdata[0] * 0.5;
			dstp[0] = static_cast<float>(f / std::pow(2, dctshift0));
			for (auto i = 1; i < sizex; ++i) {
				f = realdata[i] * coeff;
				dstp[i] = static_cast<float>(f / std::pow(2, dctshift));
			}
			dstp += dct_pitch;
			realdata += floatpitch;
			for (auto j = 1; j < sizey; ++j) {
				for (auto i = 0; i < sizex; ++i) {
					f = realdata[i] * coeff;
					dstp[i] = static_cast<float>(f / std::pow(2, dctshift));
				}
				dstp += dct_pitch;
				realdata += floatpitch;
			}
		};
		Bytes2Float();
		fftw_execute_r2r(dctplan, fSrc, fSrcDCT);
		Float2Bytes();
	}
};