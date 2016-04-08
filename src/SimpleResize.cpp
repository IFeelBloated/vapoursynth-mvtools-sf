#include <algorithm>
#include <cstdlib>
#include "SimpleResize.h"

SimpleResize::SimpleResize(int32_t _dst_width, int32_t _dst_height, int32_t _src_width, int32_t _src_height) {
	src_width = _src_width;
	src_height = _src_height;
	dst_width = _dst_width;
	dst_height = _dst_height;
	vertical_offsets = (int32_t *)malloc(dst_height * sizeof(int32_t));
	vertical_weights = (int32_t *)malloc(dst_height * sizeof(int32_t));
	horizontal_offsets = (int32_t *)malloc(dst_width * sizeof(int32_t));
	horizontal_weights = (int32_t *)malloc(dst_width * sizeof(int32_t));
	InitTables(horizontal_offsets, horizontal_weights, dst_width, src_width);
	InitTables(vertical_offsets, vertical_weights, dst_height, src_height);
};

SimpleResize::~SimpleResize() {
	free(vertical_offsets);
	free(vertical_weights);
	free(horizontal_offsets);
	free(horizontal_weights);
}

void SimpleResize::Resize(uint8_t *dstp, int32_t dst_stride, const uint8_t* srcp, int32_t src_stride) {
	const uint8_t *srcp1;
	const uint8_t *srcp2;
	uint8_t *workp = (uint8_t *)malloc(src_width);
	for (int32_t y = 0; y < dst_height; y++) {
		int32_t weight_bottom = vertical_weights[y];
		int32_t weight_top = 32768 - weight_bottom;
		srcp1 = srcp + vertical_offsets[y] * src_stride;
		srcp2 = srcp1 + src_stride;
		for (int32_t x = 0; x < src_width; x++) {
			workp[x] = (srcp1[x] * weight_top + srcp2[x] * weight_bottom + 16384) / 32768;
		}
		for (int32_t x = 0; x < dst_width; x++) {
			int32_t weight_right = horizontal_weights[x];
			int32_t weight_left = 32768 - weight_right;
			int32_t offset = horizontal_offsets[x];

			dstp[x] = (workp[offset] * weight_left + workp[offset + 1] * weight_right + 16384) / 32768;
		}
		dstp += dst_stride;
	}
	free(workp);
}

void SimpleResize::InitTables(int32_t *offsets, int32_t *weights, int32_t out, int32_t in) {
	double leftmost = 0.5;
	double rightmost = in - 0.5;
	int32_t leftmost_idx = std::max((int32_t)leftmost, 0);
	int32_t rightmost_idx = std::min((int32_t)rightmost, in - 1);
	for (int32_t i = 0; i < out; i++) {
		double position = (i + 0.5f) * (double)in / (double)out;
		double weight;
		int32_t offset;
		if (position <= leftmost) {
			offset = leftmost_idx;
			weight = 0.;
		}
		else if (position >= rightmost) {
			offset = rightmost_idx - 1;
			weight = 1.;
		}
		else {
			offset = (int32_t)(position - leftmost);
			weight = position - leftmost - offset;
		}
		offsets[i] = offset;
		weights[i] = (int32_t)(weight * 32768);
	}
}
