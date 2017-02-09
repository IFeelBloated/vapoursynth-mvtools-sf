#pragma once
#include <algorithm>
#include <cmath>
#include <mutex>
#include "fftw3.h"
#include "DCT.hpp"

static std::mutex g_fftw_plans_mutex;

class DCTFFTW final : public DCTClass {
	double *fSrc = nullptr;
	fftw_plan dctplan = nullptr;
	double *fSrcDCT = nullptr;
	int dctshift = 0;
	int dctshift0 = 0;
	template <typename PixelType>
	auto Bytes2Float(const uint8_t *srcp8, int src_pitch, double *realdata) {
		auto floatpitch = sizex;
		for (auto j = 0; j < sizey; ++j) {
			for (auto i = 0; i < sizex; ++i) {
				auto srcp = reinterpret_cast<const PixelType *>(srcp8);
				realdata[i] = srcp[i];
			}
			srcp8 += src_pitch;
			realdata += floatpitch;
		}
	}
	template <typename PixelType>
	auto Float2Bytes(uint8_t *dstp8, int dst_pitch, double *realdata) {
		auto dstp = reinterpret_cast<PixelType *>(dstp8);
		dst_pitch /= sizeof(PixelType);
		auto floatpitch = sizex;
		auto f = realdata[0] * 0.5;
		dstp[0] = static_cast<PixelType>(f / std::pow(2, dctshift0));
		for (auto i = 1; i < sizex; ++i) {
			f = realdata[i] * 0.707;
			dstp[i] = static_cast<PixelType>(f / std::pow(2, dctshift));
		}
		dstp += dst_pitch;
		realdata += floatpitch;
		for (auto j = 1; j < sizey; ++j) {
			for (auto i = 0; i < sizex; ++i) {
				f = realdata[i] * 0.707;
				dstp[i] = static_cast<PixelType>(f / std::pow(2, dctshift));
			}
			dstp += dst_pitch;
			realdata += floatpitch;
		}
	}
public:
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
		dctplan = fftw_plan_r2r_2d(sizey, sizex, fSrc, fSrcDCT, FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE); // direct fft
		g_fftw_plans_mutex.unlock();
	}
	~DCTFFTW() override {
		fftw_destroy_plan(dctplan);
		fftw_free(fSrc);
		fftw_free(fSrcDCT);
	}
	auto DCTBytes2D(const uint8_t *srcp, int src_pitch, uint8_t *dctp, int dct_pitch)->void override {
		Bytes2Float<float>(srcp, src_pitch, fSrc);
		fftw_execute_r2r(dctplan, fSrc, fSrcDCT);
		Float2Bytes<float>(dctp, dct_pitch, fSrcDCT);
	}
};