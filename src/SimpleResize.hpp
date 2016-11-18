#ifndef __SIMPLERESIZE__
#define __SIMPLERESIZE__
#include <cstdint>
#include <algorithm>

template<typename T>
class SimpleResize {
	int32_t dst_width;
	int32_t dst_height;
	int32_t src_width;
	int32_t src_height;
	int32_t *vertical_offsets;
	double *vertical_weights;
	int32_t *horizontal_offsets;
	double *horizontal_weights;
	auto InitTables(int32_t *offsets, double *weights, int32_t out, int32_t in) {
		double leftmost = 0.5;
		double rightmost = in - 0.5;
		int32_t leftmost_idx = std::max(static_cast<int32_t>(leftmost), 0);
		int32_t rightmost_idx = std::min(static_cast<int32_t>(rightmost), in - 1);
		for (auto i = 0; i < out; ++i) {
			double position = (i + 0.5) * static_cast<double>(in) / static_cast<double>(out);
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
				offset = static_cast<int32_t>(position - leftmost);
				weight = position - leftmost - offset;
			}
			offsets[i] = offset;
			weights[i] = weight;
		}
	}
public:
	SimpleResize(int32_t _dst_width, int32_t _dst_height, int32_t _src_width, int32_t _src_height) {
		src_width = _src_width;
		src_height = _src_height;
		dst_width = _dst_width;
		dst_height = _dst_height;
		vertical_offsets = new int32_t[dst_height];
		vertical_weights = new double[dst_height];
		horizontal_offsets = new int32_t[dst_width];
		horizontal_weights = new double[dst_width];
		InitTables(horizontal_offsets, horizontal_weights, dst_width, src_width);
		InitTables(vertical_offsets, vertical_weights, dst_height, src_height);
	}
	~SimpleResize() {
		delete[] vertical_offsets;
		delete[] vertical_weights;
		delete[] horizontal_offsets;
		delete[] horizontal_weights;
	}
	auto Resize(T *dstp, int32_t dst_stride, const T *srcp, int32_t src_stride) {
		const T *srcp1;
		const T *srcp2;
		auto workp = new double[src_width];
		for (auto y = 0; y < dst_height; ++y) {
			double weight_bottom = vertical_weights[y];
			double weight_top = 1. - weight_bottom;
			srcp1 = srcp + vertical_offsets[y] * src_stride;
			srcp2 = srcp1 + src_stride;
			for (auto x = 0; x < src_width; ++x)
				workp[x] = srcp1[x] * weight_top + srcp2[x] * weight_bottom;
			for (auto x = 0; x < dst_width; ++x) {
				double weight_right = horizontal_weights[x];
				double weight_left = 1. - weight_right;
				int32_t offset = horizontal_offsets[x];
				dstp[x] = static_cast<T>(workp[offset] * weight_left + workp[offset + 1] * weight_right);
			}
			dstp += dst_stride;
		}
		delete[] workp;
	}
};

#endif
