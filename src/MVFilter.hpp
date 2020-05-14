#pragma once
#include "MVClip.hpp"

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
	MVFilter(VSNodeRef *vector, const char *filterName, const VSAPI *vsapi) {
		if (vector == nullptr)
			throw MVException("vector clip must be specified"); //v1.8
		auto mvClip = MVClipDicks{ vector, 0, 0, vsapi };
		nWidth = mvClip.GetWidth();
		nHeight = mvClip.GetHeight();
		nHPadding = mvClip.GetHPadding();
		nVPadding = mvClip.GetVPadding();
		nBlkCount = mvClip.GetBlkCount();
		nBlkSizeX = mvClip.GetBlkSizeX();
		nBlkSizeY = mvClip.GetBlkSizeY();
		nBlkX = mvClip.GetBlkX();
		nBlkY = mvClip.GetBlkY();
		nPel = mvClip.GetPel();
		nOverlapX = mvClip.GetOverlapX();
		nOverlapY = mvClip.GetOverlapY();
		xRatioUV = mvClip.GetXRatioUV();
		yRatioUV = mvClip.GetYRatioUV();
		name = filterName;
	}
	void CheckSimilarity(const MVClipDicks *vector, const char *vectorName) {
		if (nWidth != vector->GetWidth())
			throw MVException(std::string(vectorName).append("'s width is incorrect."));
		if (nHeight != vector->GetHeight())
			throw MVException(std::string(vectorName).append("'s height is incorrect."));
		if (nBlkSizeX != vector->GetBlkSizeX() || nBlkSizeY != vector->GetBlkSizeY())
			throw MVException(std::string(vectorName).append("'s block size is incorrect."));
		if (nPel != vector->GetPel())
			throw MVException(std::string(vectorName).append("'s pel precision is incorrect."));
		if (nOverlapX != vector->GetOverlapX() || nOverlapY != vector->GetOverlapY())
			throw MVException(std::string(vectorName).append("'s overlap size is incorrect."));
		if (xRatioUV != vector->GetXRatioUV())
			throw MVException(std::string(vectorName).append("'s horizontal subsampling is incorrect."));
		if (yRatioUV != vector->GetYRatioUV())
			throw MVException(std::string(vectorName).append("'s vertical subsampling is incorrect."));
	}
};

