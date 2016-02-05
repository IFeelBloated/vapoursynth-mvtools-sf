#include "GroupOfPlanes.h"

GroupOfPlanes::GroupOfPlanes(int32_t _nBlkSizeX, int32_t _nBlkSizeY, int32_t _nLevelCount, int32_t _nPel, int32_t _nMotionFlags, int32_t _nOverlapX, int32_t _nOverlapY, int32_t _nBlkX, int32_t _nBlkY, int32_t _xRatioUV, int32_t _yRatioUV, int32_t _divideExtra) {
	nBlkSizeX = _nBlkSizeX;
	nBlkSizeY = _nBlkSizeY;
	nLevelCount = _nLevelCount;
	nPel = _nPel;
	nMotionFlags = _nMotionFlags;
	nOverlapX = _nOverlapX;
	nOverlapY = _nOverlapY;
	xRatioUV = _xRatioUV;
	yRatioUV = _yRatioUV;
	divideExtra = _divideExtra;
	planes = new PlaneOfBlocks*[nLevelCount];
	int32_t nBlkX = _nBlkX;
	int32_t nBlkY = _nBlkY;
	int32_t nPelCurrent = nPel;
	int32_t nMotionFlagsCurrent = nMotionFlags;
	int32_t nWidth_B = (nBlkSizeX - nOverlapX)*nBlkX + nOverlapX;
	int32_t nHeight_B = (nBlkSizeY - nOverlapY)*nBlkY + nOverlapY;
	for (int32_t i = 0; i < nLevelCount; i++) {
		if (i == nLevelCount - 1)
			nMotionFlagsCurrent |= MOTION_SMALLEST_PLANE;
		nBlkX = ((nWidth_B >> i) - nOverlapX) / (nBlkSizeX - nOverlapX);
		nBlkY = ((nHeight_B >> i) - nOverlapY) / (nBlkSizeY - nOverlapY);
		planes[i] = new PlaneOfBlocks(nBlkX, nBlkY, nBlkSizeX, nBlkSizeY, nPelCurrent, i, nMotionFlagsCurrent, nOverlapX, nOverlapY, xRatioUV, yRatioUV);
		nPelCurrent = 1;
	}
}

GroupOfPlanes::~GroupOfPlanes() {
	for (int32_t i = 0; i < nLevelCount; ++i)
		delete planes[i];
	delete[] planes;
}

void GroupOfPlanes::SearchMVs(MVGroupOfFrames *pSrcGOF, MVGroupOfFrames *pRefGOF,
	SearchType searchType, int32_t nSearchParam, int32_t nPelSearch, double nLambda,
	double lsad, int32_t pnew, int32_t plevel, bool global,
	int32_t *out, int16_t *outfilebuf, int32_t fieldShift, DCTClass * _DCT,
	int32_t pzero, int32_t pglobal, double badSAD, int32_t badrange, bool meander, int32_t *vecPrev, bool tryMany,
	SearchType coarseSearchType) {
	int32_t i;
	out[0] = GetArraySize();
	out[1] = 1;
	out += 2;
	if (vecPrev) vecPrev += 2;
	int32_t fieldShiftCur = (nLevelCount - 1 == 0) ? fieldShift : 0;
	VECTOR globalMV;
	globalMV.x = zeroMV.x;
	globalMV.y = zeroMV.y;
	globalMV.sad = zeroMV.sad;
	if (!global)
		pglobal = pzero;
	double meanLumaChange = 0.;
	SearchType searchTypeSmallest = (nLevelCount == 1 || searchType == HSEARCH || searchType == VSEARCH) ? searchType : coarseSearchType;
	int32_t nSearchParamSmallest = (nLevelCount == 1) ? nPelSearch : nSearchParam;
	bool tryManyLevel = tryMany && nLevelCount>1;
	planes[nLevelCount - 1]->SearchMVs(pSrcGOF->GetFrame(nLevelCount - 1),
		pRefGOF->GetFrame(nLevelCount - 1),
		searchTypeSmallest, nSearchParamSmallest, nLambda, lsad, pnew, plevel,
		out, &globalMV, outfilebuf, fieldShiftCur, _DCT, &meanLumaChange, divideExtra,
		pzero, pglobal, badSAD, badrange, meander, vecPrev, tryManyLevel);
	out += planes[nLevelCount - 1]->GetArraySize(divideExtra);
	if (vecPrev) vecPrev += planes[nLevelCount - 1]->GetArraySize(divideExtra);
	for (i = nLevelCount - 2; i >= 0; --i) {
		SearchType searchTypeLevel = (i == 0 || searchType == HSEARCH || searchType == VSEARCH) ? searchType : coarseSearchType;
		int32_t nSearchParamLevel = (i == 0) ? nPelSearch : nSearchParam;
		if (global)
			planes[i + 1]->EstimateGlobalMVDoubled(&globalMV);
		planes[i]->InterpolatePrediction(*(planes[i + 1]));
		fieldShiftCur = (i == 0) ? fieldShift : 0;
		tryManyLevel = tryMany && i>0;
		planes[i]->SearchMVs(pSrcGOF->GetFrame(i), pRefGOF->GetFrame(i),
			searchTypeLevel, nSearchParamLevel, nLambda, lsad, pnew, plevel,
			out, &globalMV, outfilebuf, fieldShiftCur, _DCT, &meanLumaChange, divideExtra,
			pzero, pglobal, badSAD, badrange, meander, vecPrev, tryManyLevel);
		out += int32_t(planes[i]->GetArraySize(divideExtra));
		if (vecPrev) vecPrev += planes[i]->GetArraySize(divideExtra);
	}
}

void GroupOfPlanes::RecalculateMVs(MVClipBalls &mvClip, MVGroupOfFrames *pSrcGOF, MVGroupOfFrames *pRefGOF,
	SearchType searchType, int32_t nSearchParam, double nLambda,
	int32_t pnew,
	int32_t *out, int16_t *outfilebuf, int32_t fieldShift, double thSAD, DCTClass * _DCT, int32_t smooth, bool meander) {
	out[0] = GetArraySize();
	out[1] = 1;
	out += 2;
	planes[0]->RecalculateMVs(mvClip, pSrcGOF->GetFrame(0),
		pRefGOF->GetFrame(0),
		searchType, nSearchParam, nLambda, pnew,
		out, outfilebuf, fieldShift, thSAD, _DCT, divideExtra, smooth, meander);
	out += planes[0]->GetArraySize(divideExtra);
}

void GroupOfPlanes::WriteDefaultToArray(int32_t *array) {
	array[0] = GetArraySize();
	array[1] = 0;
	array += 2;
	for (int32_t i = nLevelCount - 1; i >= 0; --i)
		array += planes[i]->WriteDefaultToArray(array, divideExtra);
}

int32_t GroupOfPlanes::GetArraySize() {
	int32_t size = 2;
	for (int32_t i = nLevelCount - 1; i >= 0; --i)
		size += planes[i]->GetArraySize(divideExtra);
	return size;
}

inline int32_t Median3(int32_t a, int32_t b, int32_t c) {
	if (((b <= a) && (a <= c)) || ((c <= a) && (a <= b))) return a;
	else if (((a <= b) && (b <= c)) || ((c <= b) && (b <= a))) return b;
	else return c;
}

void GetMedian(int32_t *vx, int32_t *vy, int32_t vx1, int32_t vy1, int32_t vx2, int32_t vy2, int32_t vx3, int32_t vy3) {
	*vx = Median3(vx1, vx2, vx3);
	*vy = Median3(vy1, vy2, vy3);
	if ((*vx == vx1 && *vy == vy1) || (*vx == vx2 && *vy == vy2) || (*vx == vx3 && *vy == vy3))
		return;
	else {
		*vx = vx1;
		*vy = vy1;
	}
}

void GroupOfPlanes::ExtraDivide(int32_t *out) {
	out += 2;
	for (int32_t i = nLevelCount - 1; i >= 1; i--)
		out += planes[i]->GetArraySize(0);
	int32_t * inp = out + 1;
	out += out[0] + 1;
	int32_t nBlkY = planes[0]->GetnBlkY();
	int32_t nBlkXN = N_PER_BLOCK*planes[0]->GetnBlkX();
	int32_t by = 0;
	for (int32_t bx = 0; bx<nBlkXN; bx += N_PER_BLOCK) {
		for (int32_t i = 0; i<2; i++) {
			out[bx * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i];
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i];
		}
		for (int32_t i = 2; i<N_PER_BLOCK; i++) {
			out[bx * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i] >> 2;
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i] >> 2;
		}
	}
	out += nBlkXN * 4;
	inp += nBlkXN;
	for (by = 1; by<nBlkY - 1; by++) {
		int32_t bx = 0;
		for (int32_t i = 0; i<2; i++) {
			out[bx * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i];
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i];
		}
		for (int32_t i = 2; i<N_PER_BLOCK; i++) {
			out[bx * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i] >> 2;
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i] >> 2;
		}
		for (bx = N_PER_BLOCK; bx<nBlkXN - N_PER_BLOCK; bx += N_PER_BLOCK) {
			if (divideExtra == 1) {
				out[bx * 2] = inp[bx];
				out[bx * 2 + N_PER_BLOCK] = inp[bx];
				out[bx * 2 + nBlkXN * 2] = inp[bx];
				out[bx * 2 + N_PER_BLOCK + nBlkXN * 2] = inp[bx];
				out[bx * 2 + 1] = inp[bx + 1];
				out[bx * 2 + N_PER_BLOCK + 1] = inp[bx + 1];
				out[bx * 2 + nBlkXN * 2 + 1] = inp[bx + 1];
				out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + 1] = inp[bx + 1];
			}
			else {
				int32_t vx;
				int32_t vy;
				GetMedian(&vx, &vy, inp[bx], inp[bx + 1], inp[bx - N_PER_BLOCK], inp[bx + 1 - N_PER_BLOCK], inp[bx - nBlkXN], inp[bx + 1 - nBlkXN]);
				out[bx * 2] = vx;
				out[bx * 2 + 1] = vy;
				GetMedian(&vx, &vy, inp[bx], inp[bx + 1], inp[bx + N_PER_BLOCK], inp[bx + 1 + N_PER_BLOCK], inp[bx - nBlkXN], inp[bx + 1 - nBlkXN]);
				out[bx * 2 + N_PER_BLOCK] = vx;
				out[bx * 2 + N_PER_BLOCK + 1] = vy;
				GetMedian(&vx, &vy, inp[bx], inp[bx + 1], inp[bx - N_PER_BLOCK], inp[bx + 1 - N_PER_BLOCK], inp[bx + nBlkXN], inp[bx + 1 + nBlkXN]);
				out[bx * 2 + nBlkXN * 2] = vx;
				out[bx * 2 + nBlkXN * 2 + 1] = vy;
				GetMedian(&vx, &vy, inp[bx], inp[bx + 1], inp[bx + N_PER_BLOCK], inp[bx + 1 + N_PER_BLOCK], inp[bx + nBlkXN], inp[bx + 1 + nBlkXN]);
				out[bx * 2 + N_PER_BLOCK + nBlkXN * 2] = vx;
				out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + 1] = vy;
			}
			for (int32_t i = 2; i<N_PER_BLOCK; i++) {
				out[bx * 2 + i] = inp[bx + i] >> 2;
				out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i] >> 2;
				out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i] >> 2;
				out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i] >> 2;
			}
		}
		bx = nBlkXN - N_PER_BLOCK;
		for (int32_t i = 0; i<2; i++) {
			out[bx * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i];
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i];
		}
		for (int32_t i = 2; i<N_PER_BLOCK; i++) {
			out[bx * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i] >> 2;
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i] >> 2;
		}
		out += nBlkXN * 4;
		inp += nBlkXN;
	}
	by = nBlkY - 1;
	for (int32_t bx = 0; bx<nBlkXN; bx += N_PER_BLOCK) {
		for (int32_t i = 0; i<2; i++) {
			out[bx * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i];
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i];
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i];
		}
		for (int32_t i = 2; i<N_PER_BLOCK; i++) {
			out[bx * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + i] = inp[bx + i] >> 2;
			out[bx * 2 + nBlkXN * 2 + i] = inp[bx + i] >> 2;
			out[bx * 2 + N_PER_BLOCK + nBlkXN * 2 + i] = inp[bx + i] >> 2;
		}
	}
}
