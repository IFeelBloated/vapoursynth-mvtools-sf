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
	self(limit_width, 0);
	self(limit_height, 0);
	self(pel, 0);
	int32_t *vertical_offsets;
	double *vertical_weights;
	int32_t *horizontal_offsets;
	double *horizontal_weights;
	auto InitTables(int32_t *offsets, double *weights, int32_t out, int32_t in) {
		auto leftmost = 0.5;
		auto rightmost = in - 0.5;
		auto leftmost_idx = std::max(static_cast<std::int32_t>(leftmost), 0);
		auto rightmost_idx = std::min(static_cast<std::int32_t>(rightmost), in - 1);
		for (auto i = 0; i < out; ++i) {
			auto position = (i + 0.5) * in / out;
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
	SimpleResize(int32_t _dst_width, int32_t _dst_height, int32_t _src_width, int32_t _src_height, auto limit_width, auto limit_height, auto pel) {
		src_width = _src_width;
		src_height = _src_height;
		dst_width = _dst_width;
		dst_height = _dst_height;
		this->limit_width = limit_width;
		this->limit_height = limit_height;
		this->pel = pel;
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
	auto Resize(T *dstp, int32_t dst_stride, const T *srcp, int32_t src_stride, auto horizontal_vectors) {
		constexpr auto limit_vectors = requires(T x) { x << 0; };
		auto minimum = static_cast<T>(0);
		auto maximum = static_cast<T>(limit_height * pel - 1);
		auto horizontal_step = horizontal_vectors ? pel : 0;
		auto vertical_step = horizontal_vectors ? 0 : pel;


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

			if (horizontal_vectors) {
				minimum = 0;
				maximum = limit_width * pel - 1;
			}

			for (auto x = 0; x < dst_width; ++x) {
				auto weight_right = horizontal_weights[x];
				auto weight_left = 1. - weight_right;
				auto offset = horizontal_offsets[x];
				auto result = static_cast<T>(workp[offset] * weight_left + workp[offset + 1] * weight_right);

				if constexpr (limit_vectors) {
					result = std::max(minimum, std::min(result, maximum));
					minimum -= horizontal_step;
					maximum -= horizontal_step;
				}

				dstp[x] = result;
			}
			dstp += dst_stride;

			if constexpr (limit_vectors) {
				minimum -= vertical_step;
				maximum -= vertical_step;
			}
		}
		delete[] workp;
	}
};

#endif
