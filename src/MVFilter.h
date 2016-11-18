#ifndef MVTOOLS_MVFILTER_H
#define MVTOOLS_MVFILTER_H
#include "MVClip.h"

class MVFilter {
public:
	int32_t nBlkX;
	int32_t nBlkY;
	int32_t nBlkCount;
	int32_t nBlkSizeX;
	int32_t nBlkSizeY;
	int32_t nHPadding;
	int32_t nVPadding;
	int32_t nWidth;
	int32_t nHeight;
	int32_t nIdx;
	int32_t nPel;
	int32_t nOverlapX;
	int32_t nOverlapY;
	int32_t yRatioUV;
	int32_t xRatioUV;
	const char * name;
	MVFilter(VSNodeRef *vector, const char *filterName, const VSAPI *vsapi);
	void CheckSimilarity(const MVClipDicks *vector, const char *vectorName);
};

#endif

