#ifndef __MV_DCT__
#define __MV_DCT__
#include <cstdint>

class DCTClass {
public:
	DCTClass(int32_t _sizex, int32_t _sizey, int32_t _dctmode);
	virtual ~DCTClass();
	virtual void DCTBytes2D(const uint8_t *srcp0, int32_t _src_pitch,
		uint8_t *dctp, int32_t _dct_pitch) = 0;
	int32_t sizex;
	int32_t sizey;
	int32_t dctmode;
};

#endif
