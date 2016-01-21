#ifndef __GOPLANES__
#define __GOPLANES__
#include "PlaneOfBlocks.h"

class GroupOfPlanes {
	int nBlkSizeX;
	int nBlkSizeY;
	int nLevelCount;
	int nPel;
	int nMotionFlags;
	int nOverlapX;
	int nOverlapY;
	int xRatioUV;
	int yRatioUV;
	int divideExtra;
	PlaneOfBlocks **planes;
public:
	GroupOfPlanes(int _nBlkSizeX, int _nBlkSizeY, int _nLevelCount, int _nPel,
		int _nMotionFlags, int _nOverlapX, int _nOverlapY, int _nBlkX, int _nBlkY, int _xRatioUV, int _yRatioUV, int _divideExtra);
	~GroupOfPlanes();
	void SearchMVs(MVGroupOfFrames *pSrcGOF, MVGroupOfFrames *pRefGOF,
		SearchType searchType, int nSearchParam, int _PelSearch, double _nLambda, double _lsad, int _pnew, int _plevel, bool _global, int *out, short * outfilebuf, int fieldShift, DCTClass * DCT, int _pzero, int _pglobal, double badSAD, int badrange, bool meander, int *vecPrev, bool tryMany, SearchType coarseSearchType);
	void WriteDefaultToArray(int *array);
	int GetArraySize();
	void ExtraDivide(int *out);
	void RecalculateMVs(MVClipBalls &mvClip, MVGroupOfFrames *pSrcGOF, MVGroupOfFrames *pRefGOF,
		SearchType _searchType, int _nSearchParam, float _nLambda, int _pnew, int *out, short * outfilebuf, int fieldShift, float thSAD, DCTClass * DCT, int smooth, bool meander);
};

#endif
