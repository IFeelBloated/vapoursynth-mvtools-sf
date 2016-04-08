#include "FakePlaneOfBlocks.h"
#include "SADFunctions.h"
#include "CommonFunctions.h"

FakePlaneOfBlocks::FakePlaneOfBlocks(int32_t sizeX, int32_t sizeY, int32_t lv, int32_t pel, int32_t _nOverlapX, int32_t _nOverlapY, int32_t _nBlkX, int32_t _nBlkY) {
	nBlkSizeX = sizeX;
	nBlkSizeY = sizeY;
	nOverlapX = _nOverlapX;
	nOverlapY = _nOverlapY;
	nBlkX = _nBlkX;
	nBlkY = _nBlkY;
	nWidth_Bi = nOverlapX + nBlkX*(nBlkSizeX - nOverlapX);
	nHeight_Bi = nOverlapY + nBlkY*(nBlkSizeY - nOverlapY);
	nBlkCount = nBlkX * nBlkY;
	nPel = pel;
	nLogPel = ilog2(nPel);
	nLogScale = lv;
	nScale = iexp2(nLogScale);
	blocks = new FakeBlockData[nBlkCount];
	for (int32_t j = 0, blkIdx = 0; j < nBlkY; j++)
		for (int32_t i = 0; i < nBlkX; i++, blkIdx++)
			blocks[blkIdx].Init(i * (nBlkSizeX - nOverlapX), j * (nBlkSizeY - nOverlapY));
}

FakePlaneOfBlocks::~FakePlaneOfBlocks() {
	delete[] blocks;
}

void FakePlaneOfBlocks::Update(const int32_t *array) {
	array += 0;
	for (int32_t i = 0; i < nBlkCount; i++) {
		blocks[i].Update(array);
		array += N_PER_BLOCK;
	}
}

bool FakePlaneOfBlocks::IsSceneChange(double nTh1, double nTh2) const {
	int32_t sum = 0;
	for (int32_t i = 0; i < nBlkCount; i++)
		sum += (_back2flt(blocks[i].GetSAD()) > nTh1) ? 1 : 0;
	return (sum > nTh2);
}
