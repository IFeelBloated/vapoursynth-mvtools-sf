#ifndef __MV_SUPER__
#define __MV_SUPER__

inline int PlaneHeightLuma(int src_height, int level, int yRatioUV, int vpad) {
	int height = src_height;
	for (int i = 1; i <= level; i++)
		height = vpad >= yRatioUV ? ((height / yRatioUV + 1) / 2) * yRatioUV : ((height / yRatioUV) / 2) * yRatioUV;
	return height;
}

inline int PlaneWidthLuma(int src_width, int level, int xRatioUV, int hpad) {
	int width = src_width;
	for (int i = 1; i <= level; i++)
		width = hpad >= xRatioUV ? ((width / xRatioUV + 1) / 2) * xRatioUV : ((width / xRatioUV) / 2) * xRatioUV;
	return width;
}

inline unsigned int PlaneSuperOffset(bool chroma, int src_height, int level, int pel, int vpad, int plane_pitch, int yRatioUV) {
	int height = src_height;
	unsigned int offset;
	if (level == 0)
		offset = 0;
	else {
		offset = pel*pel*plane_pitch*(src_height + vpad * 2);
		for (int i = 1; i<level; i++) {
			height = chroma ? PlaneHeightLuma(src_height*yRatioUV, i, yRatioUV, vpad*yRatioUV) / yRatioUV : PlaneHeightLuma(src_height, i, yRatioUV, vpad);
			offset += plane_pitch*(height + vpad * 2);
		}
	}
	return offset;
}

#endif
