#ifndef __MV_DCTFFTW__
#define __MV_DCTFFTW__
#include "fftw3.h"
#include "DCT.h"

class DCTFFTW : public DCTClass {
	double *fSrc;
	fftw_plan dctplan;
	double *fSrcDCT;
	int32_t dctshift;
	int32_t dctshift0;
	template <typename PixelType>
	void Bytes2Float(const uint8_t *srcp0, int32_t _pitch, double *realdata);
	template <typename PixelType>
	void Float2Bytes(uint8_t *srcp0, int32_t _pitch, double *realdata);
public:
	DCTFFTW(int32_t _sizex, int32_t _sizey, int32_t _dctmode);
	~DCTFFTW();
	virtual void DCTBytes2D(const uint8_t *srcp0, int32_t _src_pitch,
		uint8_t *dctp, int32_t _dct_pitch) override;
};

#endif
