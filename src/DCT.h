#ifndef __MV_DCT__
#define __MV_DCT__
#include <cstdint>

class DCTClass {
public:
	DCTClass(int _sizex, int _sizey, int _dctmode);
	virtual ~DCTClass();
	virtual void DCTBytes2D(const uint8_t *srcp0, int _src_pitch,
		uint8_t *dctp, int _dct_pitch) = 0;
	int sizex;
	int sizey;
	int dctmode;
};

#endif
