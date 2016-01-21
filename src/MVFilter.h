#ifndef MVTOOLS_MVFILTER_H
#define MVTOOLS_MVFILTER_H
#include "MVClip.h"

class MVFilter {
public:
	int nBlkX;
	int nBlkY;
	int nBlkCount;
	int nBlkSizeX;
	int nBlkSizeY;
	int nHPadding;
	int nVPadding;
	int nWidth;
	int nHeight;
	int nIdx;
	int nPel;
	int nOverlapX;
	int nOverlapY;
	int yRatioUV;
	int xRatioUV;
	const char * name;
	MVFilter(VSNodeRef *vector, const char *filterName, const VSAPI *vsapi);
	void CheckSimilarity(const MVClipDicks *vector, const char *vectorName);
};

#endif

