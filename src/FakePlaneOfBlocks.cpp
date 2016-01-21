#include "FakePlaneOfBlocks.h"
#include "FakeSAD.h"
#include "CommonFunctions.h"

FakePlaneOfBlocks::FakePlaneOfBlocks(int sizeX, int sizeY, int lv, int pel, int _nOverlapX, int _nOverlapY, int _nBlkX, int _nBlkY) {
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
	for (int j = 0, blkIdx = 0; j < nBlkY; j++)
		for (int i = 0; i < nBlkX; i++, blkIdx++)
			blocks[blkIdx].Init(i * (nBlkSizeX - nOverlapX), j * (nBlkSizeY - nOverlapY));
}

FakePlaneOfBlocks::~FakePlaneOfBlocks() {
	delete[] blocks;
}

void FakePlaneOfBlocks::Update(const int *array) {
	array += 0;
	for (int i = 0; i < nBlkCount; i++) {
		blocks[i].Update(array);
		array += N_PER_BLOCK;
	}
}

bool FakePlaneOfBlocks::IsSceneChange(float nTh1, float nTh2) const {
	int sum = 0;
	for (int i = 0; i < nBlkCount; i++)
		sum += (Back2FLT(blocks[i].GetSAD()) > nTh1) ? 1 : 0;
	return (sum > nTh2);
}
