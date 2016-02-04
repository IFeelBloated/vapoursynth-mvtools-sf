#include <algorithm>
#include <cmath>
#include <mutex>
#include "DCTFFTW.h"
std::mutex g_fftw_plans_mutex;

DCTFFTW::DCTFFTW(int _sizex, int _sizey, int _dctmode) :
	DCTClass(_sizex, _sizey, _dctmode) {
	int size2d = sizey*sizex;
	int cursize = 1;
	dctshift = 0;
	while (cursize < size2d) {
		dctshift++;
		cursize = (cursize << 1);
	}
	dctshift0 = dctshift + 2;
	fSrc = (double *)fftw_malloc(sizeof(double) * size2d);
	fSrcDCT = (double *)fftw_malloc(sizeof(double) * size2d);
	g_fftw_plans_mutex.lock();
	dctplan = fftw_plan_r2r_2d(sizey, sizex, fSrc, fSrcDCT,
		FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE); // direct fft
	g_fftw_plans_mutex.unlock();
}

DCTFFTW::~DCTFFTW() {
	fftw_destroy_plan(dctplan);
	fftw_free(fSrc);
	fftw_free(fSrcDCT);
}

template <typename PixelType>
void DCTFFTW::Bytes2Float(const uint8_t *srcp8, int src_pitch, double *realdata) {
	int floatpitch = sizex;
	int i, j;
	for (j = 0; j < sizey; ++j) {
		for (i = 0; i < sizex; i += 1) {
			PixelType *srcp = reinterpret_cast<PixelType *>(const_cast<uint8_t *>(srcp8));
			realdata[i] = static_cast<double>(srcp[i]);
		}
		srcp8 += src_pitch;
		realdata += floatpitch;
	}
}

template <typename PixelType>
void DCTFFTW::Float2Bytes(uint8_t *dstp8, int dst_pitch, double *realdata) {
	PixelType *dstp = reinterpret_cast<PixelType *>(dstp8);
	dst_pitch /= sizeof(PixelType);
	double pixelMax = 1.0;
	double pixelHalf = 0.5;
	int floatpitch = sizex;
	int i, j;
	double f = realdata[0] * 0.5;
	dstp[0] = static_cast<float>(std::min(pixelMax, std::max(0.0, (f / pow(2, dctshift0)) + pixelHalf)));
	for (i = 1; i < sizex; i += 1) {
		f = realdata[i] * 0.707;
		dstp[i] = static_cast<float>(std::min(pixelMax, std::max(0.0, (f / pow(2, dctshift)) + pixelHalf)));
	}
	dstp += dst_pitch;
	realdata += floatpitch;
	for (j = 1; j < sizey; j++) {
		for (i = 0; i < sizex; i += 1) {
			f = realdata[i] * 0.707;
			dstp[i] = static_cast<float>(std::min(pixelMax, std::max(0.0, (f / pow(2, dctshift)) + pixelHalf)));
		}
		dstp += dst_pitch;
		realdata += floatpitch;
	}
}

void DCTFFTW::DCTBytes2D(const uint8_t *srcp, int src_pitch, uint8_t *dctp, int dct_pitch) {
	Bytes2Float<float>(srcp, src_pitch, fSrc);
	fftw_execute_r2r(dctplan, fSrc, fSrcDCT);
	Float2Bytes<float>(dctp, dct_pitch, fSrcDCT);
}
