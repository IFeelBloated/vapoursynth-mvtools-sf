#ifndef __GOPLANES__
#define __GOPLANES__
#include "PlaneOfBlocks.h"

class GroupOfPlanes {
	int32_t nBlkSizeX;
	int32_t nBlkSizeY;
	int32_t nLevelCount;
	int32_t nPel;
	int32_t nMotionFlags;
	int32_t nOverlapX;
	int32_t nOverlapY;
	int32_t xRatioUV;
	int32_t yRatioUV;
	int32_t divideExtra;
	PlaneOfBlocks **planes;
public:
	GroupOfPlanes(int32_t _nBlkSizeX, int32_t _nBlkSizeY, int32_t _nLevelCount, int32_t _nPel,
		int32_t _nMotionFlags, int32_t _nOverlapX, int32_t _nOverlapY, int32_t _nBlkX, int32_t _nBlkY, int32_t _xRatioUV, int32_t _yRatioUV, int32_t _divideExtra);
	~GroupOfPlanes();
	void SearchMVs(MVGroupOfFrames *pSrcGOF, MVGroupOfFrames *pRefGOF,
		SearchType searchType, int32_t nSearchParam, int32_t _PelSearch, double _nLambda, double _lsad, int32_t _pnew, int32_t _plevel, bool _global, int32_t *out, int32_t * outfilebuf, int32_t fieldShift, DCTClass * DCT, int32_t _pzero, int32_t _pglobal, double badSAD, int32_t badrange, bool meander, int32_t *vecPrev, bool tryMany, SearchType coarseSearchType);
	void WriteDefaultToArray(int32_t *array);
	int32_t GetArraySize();
	void ExtraDivide(int32_t *out);
	void RecalculateMVs(MVClipBalls &mvClip, MVGroupOfFrames *pSrcGOF, MVGroupOfFrames *pRefGOF,
		SearchType _searchType, int32_t _nSearchParam, double _nLambda, int32_t _pnew, int32_t *out, int32_t * outfilebuf, int32_t fieldShift, double thSAD, DCTClass * DCT, int32_t smooth, bool meander);
};

#endif
