#ifndef __SIMPLERESIZE__
#define __SIMPLERESIZE__
#include <cstdint>

class SimpleResize {
	int dst_width;
	int dst_height;
	int src_width;
	int src_height;
	int *vertical_offsets;
	int *vertical_weights;
	int *horizontal_offsets;
	int *horizontal_weights;
	void InitTables(int *offsets, int *weights, int out, int in);
public:
	SimpleResize(int _dst_width, int _dst_height, int _src_width, int _src_height);
	~SimpleResize();
	void Resize(uint8_t *dstp, int dst_stride, const uint8_t* srcp, int src_stride);
};

#endif
