#ifndef __MV_SUPER__
#define __MV_SUPER__

inline int32_t PlaneHeightLuma(int32_t src_height, int32_t level, int32_t yRatioUV, int32_t vpad) {
	int32_t height = src_height;
	for (int32_t i = 1; i <= level; i++)
		height = vpad >= yRatioUV ? ((height / yRatioUV + 1) / 2) * yRatioUV : ((height / yRatioUV) / 2) * yRatioUV;
	return height;
}

inline int32_t PlaneWidthLuma(int32_t src_width, int32_t level, int32_t xRatioUV, int32_t hpad) {
	int32_t width = src_width;
	for (int32_t i = 1; i <= level; i++)
		width = hpad >= xRatioUV ? ((width / xRatioUV + 1) / 2) * xRatioUV : ((width / xRatioUV) / 2) * xRatioUV;
	return width;
}

inline uint32_t PlaneSuperOffset(bool chroma, int32_t src_height, int32_t level, int32_t pel, int32_t vpad, int32_t plane_pitch, int32_t yRatioUV) {
	int32_t height = src_height;
	uint32_t offset;
	if (level == 0)
		offset = 0;
	else {
		offset = pel*pel*plane_pitch*(src_height + vpad * 2);
		for (int32_t i = 1; i<level; i++) {
			height = chroma ? PlaneHeightLuma(src_height*yRatioUV, i, yRatioUV, vpad*yRatioUV) / yRatioUV : PlaneHeightLuma(src_height, i, yRatioUV, vpad);
			offset += plane_pitch*(height + vpad * 2);
		}
	}
	return offset;
}

#endif
