#ifndef MVTOOLS_FAKEPLANEOFBLOCKS_H
#define MVTOOLS_FAKEPLANEOFBLOCKS_H
#include "FakeBlockData.h"

class FakePlaneOfBlocks {
	int32_t nWidth_Bi;
	int32_t nHeight_Bi;
	int32_t nBlkX;
	int32_t nBlkY;
	int32_t nBlkSizeX;
	int32_t nBlkSizeY;
	int32_t nBlkCount;
	int32_t nPel;
	int32_t nLogPel;
	int32_t nScale;
	int32_t nLogScale;
	int32_t nOverlapX;
	int32_t nOverlapY;
	FakeBlockData *blocks;
public:
	FakePlaneOfBlocks(int32_t sizex, int32_t sizey, int32_t lv, int32_t pel, int32_t overlapx, int32_t overlapy, int32_t nBlkX, int32_t nBlkY);
	~FakePlaneOfBlocks();
	void Update(const int32_t *array);
	bool IsSceneChange(double nTh1, double nTh2) const;
	inline bool IsInFrame(int32_t i) const {
		return ((i >= 0) && (i < nBlkCount));
	}
	inline const FakeBlockData& operator[](const int32_t i) const {
		return (blocks[i]);
	}
	inline int32_t GetBlockCount() const { return nBlkCount; }
	inline int32_t GetReducedWidth() const { return nBlkX; }
	inline int32_t GetReducedHeight() const { return nBlkY; }
	inline int32_t GetWidth() const { return nWidth_Bi; }
	inline int32_t GetHeight() const { return nHeight_Bi; }
	inline int32_t GetScaleLevel() const { return nLogScale; }
	inline int32_t GetEffectiveScale() const { return nScale; }
	inline int32_t GetBlockSizeX() const { return nBlkSizeX; }
	inline int32_t GetBlockSizeY() const { return nBlkSizeY; }
	inline int32_t GetPel() const { return nPel; }
	inline const FakeBlockData& GetBlock(int32_t i) const { return (blocks[i]); }
	inline int32_t GetOverlapX() const { return nOverlapX; }
	inline int32_t GetOverlapY() const { return nOverlapY; }
};

#endif

