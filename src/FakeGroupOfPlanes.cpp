#include "FakeGroupOfPlanes.h"

void FakeGroupOfPlanes::Create(int32_t nBlkSizeX, int32_t nBlkSizeY, int32_t nLevelCount, int32_t nPel, int32_t nOverlapX, int32_t nOverlapY, int32_t _yRatioUV, int32_t _nBlkX, int32_t _nBlkY) {
	nLvCount_ = nLevelCount;
	int32_t nBlkX1 = _nBlkX;
	int32_t nBlkY1 = _nBlkY;
	nWidth_B = (nBlkSizeX - nOverlapX)*nBlkX1 + nOverlapX;
	nHeight_B = (nBlkSizeY - nOverlapY)*nBlkY1 + nOverlapY;
	yRatioUV_B = _yRatioUV;
	planes = new FakePlaneOfBlocks*[nLevelCount];
	planes[0] = new FakePlaneOfBlocks(nBlkSizeX, nBlkSizeY, 0, nPel, nOverlapX, nOverlapY, nBlkX1, nBlkY1);
	for (int32_t i = 1; i < nLevelCount; i++) {
		nBlkX1 = ((nWidth_B >> i) - nOverlapX) / (nBlkSizeX - nOverlapX);
		nBlkY1 = ((nHeight_B >> i) - nOverlapY) / (nBlkSizeY - nOverlapY);
		planes[i] = new FakePlaneOfBlocks(nBlkSizeX, nBlkSizeY, i, 1, nOverlapX, nOverlapY, nBlkX1, nBlkY1);
	}
}

FakeGroupOfPlanes::FakeGroupOfPlanes() {
	planes = 0;
}

FakeGroupOfPlanes::~FakeGroupOfPlanes() {
	if (planes) {
		for (int32_t i = 0; i < nLvCount_; i++)
			delete planes[i];
		delete[] planes;
		planes = 0;
	}
}

void FakeGroupOfPlanes::Update(const int32_t *array) {
	const int32_t *pA = array;
	validity = GetValidity(array);
	pA += 2;
	for (int32_t i = nLvCount_ - 1; i >= 0; i--)
		pA += pA[0];
	pA++;
	pA = array;
	pA += 2;
	for (int32_t i = nLvCount_ - 1; i >= 0; i--) {
		planes[i]->Update(pA + 1);
		pA += pA[0];
	}
}

bool FakeGroupOfPlanes::IsSceneChange(double nThSCD1, double nThSCD2) const {
	return planes[0]->IsSceneChange(nThSCD1, nThSCD2);
}
