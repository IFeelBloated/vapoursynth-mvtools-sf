#ifndef __SIMPLERESIZE__
#define __SIMPLERESIZE__
#include <cstdint>

class SimpleResize {
	int32_t dst_width;
	int32_t dst_height;
	int32_t src_width;
	int32_t src_height;
	int32_t *vertical_offsets;
	int32_t *vertical_weights;
	int32_t *horizontal_offsets;
	int32_t *horizontal_weights;
	void InitTables(int32_t *offsets, int32_t *weights, int32_t out, int32_t in);
public:
	SimpleResize(int32_t _dst_width, int32_t _dst_height, int32_t _src_width, int32_t _src_height);
	~SimpleResize();
	void Resize(uint8_t *dstp, int32_t dst_stride, const uint8_t* srcp, int32_t src_stride);
};

#endif
