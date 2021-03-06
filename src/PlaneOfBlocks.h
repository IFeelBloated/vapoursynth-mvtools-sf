#pragma once
#include <cstdlib>
#include <cmath>
#include <array>
#include "MVClip.hpp"
#include "MVFrame.h"
#include "Interpolation.h"
#include "CopyCode.hpp"
#include "SADFunctions.hpp"
#include "CommonFunctions.h"
#include "Variance.hpp"
#include "DCT.hpp"
#include "Padding.h"
#include "VSHelper.h"

class PlaneOfBlocks {
	static constexpr auto MAX_PREDICTOR = 20;
	int32_t nBlkX;
	int32_t nBlkY;
	int32_t nBlkSizeX;
	int32_t nBlkSizeY;
	int32_t nBlkCount;
	int32_t nPel;
	int32_t nLogPel;
	int32_t nScale;
	int32_t nLogScale;
	int32_t nMotionFlags;
	int32_t nOverlapX;
	int32_t nOverlapY;
	int32_t xRatioUV;
	int32_t yRatioUV;
	int32_t nLogxRatioUV;
	int32_t nLogyRatioUV;
	SADFunction SAD;
	LUMAFunction LUMA;
	COPYFunction BLITLUMA;
	COPYFunction BLITCHROMA;
	SADFunction SADCHROMA;
	SADFunction SATD;
	VectorStructure* vectors;
	bool smallestPlane;
	bool chroma;
	MVFrame* pSrcFrame;
	MVFrame* pRefFrame;
	int32_t nSrcPitch[3];
	const uint8_t* pSrc[3];
	int32_t nRefPitch[3];
	VectorStructure bestMV;
	int32_t nBestSad;
	double nMinCost;
	VectorStructure predictor;
	VectorStructure predictors[MAX_PREDICTOR];
	int32_t nDxMin;
	int32_t nDyMin;
	int32_t nDxMax;
	int32_t nDyMax;
	int32_t x[3];
	int32_t y[3];
	int32_t blkx;
	int32_t blky;
	int32_t blkIdx;
	int32_t blkScanDir;
	SearchType searchType;
	int32_t nSearchParam;
	double nLambda;
	double LSAD;
	int32_t penaltyNew;
	int32_t penaltyZero;
	int32_t pglobal;
	double badSAD;
	int32_t badrange;
	double planeSAD;
	int32_t badcount;
	bool temporal;
	bool tryMany;
	int32_t iter;
	VectorStructure globalMVPredictor;
	VectorStructure zeroMVfieldShifted;
	DCTClass* DCT;
	uint8_t* dctSrc;
	uint8_t* dctRef;
	int32_t dctpitch;
	int32_t dctmode;
	double srcLuma;
	double refLuma;
	double sumLumaChange;
	double dctweight16;
	int32_t* freqArray;
	int32_t freqSize;
	double verybigSAD;
	int32_t nSrcPitch_temp[3];
	uint8_t* pSrc_temp[3];
	inline const uint8_t* GetRefBlock(int32_t nVx, int32_t nVy) {
		if (nPel == 2)
			return pRefFrame->GetPlane(YPLANE)->GetAbsolutePointerPel2(
				x[0] * 2 + nVx,
				y[0] * 2 + nVy);
		else if (nPel == 1)
			return pRefFrame->GetPlane(YPLANE)->GetAbsolutePointerPel1(
				x[0] + nVx,
				y[0] + nVy);
		else
			return pRefFrame->GetPlane(YPLANE)->GetAbsolutePointerPel4(
				x[0] * 4 + nVx,
				y[0] * 4 + nVy);
	}
	inline const uint8_t* GetRefBlockU(int32_t nVx, int32_t nVy) {
		if (nPel == 2)
			return pRefFrame->GetPlane(UPLANE)->GetAbsolutePointerPel2(
				x[1] * 2 + nVx / xRatioUV,
				y[1] * 2 + nVy / yRatioUV);
		else if (nPel == 1)
			return pRefFrame->GetPlane(UPLANE)->GetAbsolutePointerPel1(
				x[1] + nVx / xRatioUV,
				y[1] + nVy / yRatioUV);
		else
			return pRefFrame->GetPlane(UPLANE)->GetAbsolutePointerPel4(
				x[1] * 4 + nVx / xRatioUV,
				y[1] * 4 + nVy / yRatioUV);
	}
	inline const uint8_t* GetRefBlockV(int32_t nVx, int32_t nVy) {
		if (nPel == 2)
			return pRefFrame->GetPlane(VPLANE)->GetAbsolutePointerPel2(
				x[2] * 2 + nVx / xRatioUV,
				y[2] * 2 + nVy / yRatioUV);
		else if (nPel == 1)
			return pRefFrame->GetPlane(VPLANE)->GetAbsolutePointerPel1(
				x[2] + nVx / xRatioUV,
				y[2] + nVy / yRatioUV);
		else
			return pRefFrame->GetPlane(VPLANE)->GetAbsolutePointerPel4(
				x[2] * 4 + nVx / xRatioUV,
				y[2] * 4 + nVy / yRatioUV);
	}
	inline const uint8_t* GetSrcBlock(int32_t nX, int32_t nY) {
		return pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(nX, nY);
	}
	inline double MotionDistorsion(int32_t vx, int32_t vy) {
		int32_t dist = SquareDifferenceNorm(predictor, vx, vy);
		return (nLambda * dist) / 256.;
	}
	double LumaSADx(const uint8_t* pRef0) {
		double sad;
		switch (dctmode) {
		case 1:
			DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
			{
				float* dctSrc16 = (float*)dctSrc;
				float* dctRef16 = (float*)dctRef;
				sad = (SAD(dctSrc, dctpitch, dctRef, dctpitch) + std::abs(dctSrc16[0] - dctRef16[0]) * 3) * nBlkSizeX / 2;
			}
			break;
		case 2:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 0) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				double dctsad;
				{
					float* dctSrc16 = (float*)dctSrc;
					float* dctRef16 = (float*)dctRef;
					dctsad = (SAD(dctSrc, dctpitch, dctRef, dctpitch) + std::abs(dctSrc16[0] - dctRef16[0]) * 3) * nBlkSizeX / 2;
				}
				sad = (sad * (16 - dctweight16) + dctsad * dctweight16) / 16;
			}
			break;
		case 3:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (std::abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				double dctsad = SAD(dctSrc, dctpitch, dctRef, dctpitch) * nBlkSizeX / 2;
				sad = sad / 2 + dctsad / 2;
			}
			break;
		case 4:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (std::abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				double dctsad = SAD(dctSrc, dctpitch, dctRef, dctpitch) * nBlkSizeX / 2;
				sad = sad / 4 + dctsad / 2 + dctsad / 4;
			}
			break;
		case 5:
			sad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			break;
		case 6:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 0) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = (sad * (16 - dctweight16) + dctsad * dctweight16) / 16;
			}
			break;
		case 7:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (std::abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 2 + dctsad / 2;
			}
			break;
		case 8:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (std::abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 4 + dctsad / 2 + dctsad / 4;
			}
			break;
		case 9:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 1) {
				double dctweighthalf = dctweight16 / 2;
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = (sad * (16 - dctweighthalf) + dctsad * dctweighthalf) / 16;
			}
			break;
		case 10:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (std::abs(srcLuma - refLuma) > (srcLuma + refLuma) / 16) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 2 + dctsad / 4 + sad / 4;
			}
			break;
		default:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
		}
		return sad;
	}
	inline double LumaSAD(const uint8_t* pRef0) {
		return !dctmode ? SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]) : LumaSADx(pRef0);
	}
	inline void CheckMV0(int32_t vx, int32_t vy) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad;
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv;
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = sad + saduv;
		}
	}
	inline void CheckMV(int32_t vx, int32_t vy) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew * sad) / 256);
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew * saduv) / 256);
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = sad + saduv;
		}
	}
	inline void CheckMV2(int32_t vx, int32_t vy, int32_t* dir, int32_t val) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew * sad) / 256);
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew * saduv) / 256);
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = sad + saduv;
			*dir = val;
		}
	}
	inline void CheckMVdir(int32_t vx, int32_t vy, int32_t* dir, int32_t val) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew * sad) / 256);
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew * saduv) / 256);
			if (cost >= nMinCost) return;
			nMinCost = cost;
			bestMV.sad = sad + saduv;
			*dir = val;
		}
	}
	inline int32_t ClipMVx(int32_t vx) {
		if (vx < nDxMin) return nDxMin;
		else if (vx >= nDxMax) return nDxMax - 1;
		else return vx;
	}
	inline int32_t ClipMVy(int32_t vy) {
		if (vy < nDyMin) return nDyMin;
		else if (vy >= nDyMax) return nDyMax - 1;
		else return vy;
	}
	inline VectorStructure ClipMV(VectorStructure v) {
		VectorStructure v2;
		v2.x = ClipMVx(v.x);
		v2.y = ClipMVy(v.y);
		v2.sad = v.sad;
		return v2;
	}
	static inline int32_t Median(int32_t a, int32_t b, int32_t c) {
		if (a < b) {
			if (b < c) return b;
			else if (a < c) return c;
			else return a;
		}
		else {
			if (a < c) return a;
			else if (b < c) return c;
			else return b;
		}
	}
	bool IsVectorOK(int32_t vx, int32_t vy) {
		return ((vx >= nDxMin) &&
			(vy >= nDyMin) &&
			(vx < nDxMax) &&
			(vy < nDyMax));
	}
	static inline uint32_t SquareDifferenceNorm(const VectorStructure& v1, const VectorStructure& v2) {
		return (v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y);
	}
	static inline uint32_t SquareDifferenceNorm(const VectorStructure& v1, const int32_t v2x, const int32_t v2y) {
		return (v1.x - v2x) * (v1.x - v2x) + (v1.y - v2y) * (v1.y - v2y);
	}
	inline bool IsInFrame(int32_t i) {
		return ((i >= 0) && (i < nBlkCount));
	}
	void Refine() {
		// then, we refine, according to the search type
		if (searchType & ONETIME)
			for (int32_t i = nSearchParam; i > 0; i /= 2)
				OneTimeSearch(i);

		if (searchType & NSTEP)
			NStepSearch(nSearchParam);

		if (searchType & LOGARITHMIC)
			for (int32_t i = nSearchParam; i > 0; i /= 2)
				DiamondSearch(i);

		if (searchType & EXHAUSTIVE)
		{
			int32_t mvx = bestMV.x;
			int32_t mvy = bestMV.y;
			for (int32_t i = 1; i <= nSearchParam; i++)// region is same as enhausted, but ordered by radius (from near to far)
				ExpandingSearch(i, 1, mvx, mvy);
		}

		if (searchType & HEX2SEARCH)
			Hex2Search(nSearchParam);

		if (searchType & UMHSEARCH)
			UMHSearch(nSearchParam, bestMV.x, bestMV.y);

		if (searchType & HSEARCH)
		{
			int32_t mvx = bestMV.x;
			int32_t mvy = bestMV.y;
			for (int32_t i = 1; i <= nSearchParam; i++)
			{
				CheckMV(mvx - i, mvy);
				CheckMV(mvx + i, mvy);
			}
		}

		if (searchType & VSEARCH)
		{
			int32_t mvx = bestMV.x;
			int32_t mvy = bestMV.y;
			for (int32_t i = 1; i <= nSearchParam; i++)
			{
				CheckMV(mvx, mvy - i);
				CheckMV(mvx, mvy + i);
			}
		}
	}
public:
	PlaneOfBlocks(int32_t _nBlkX, int32_t _nBlkY, int32_t _nBlkSizeX, int32_t _nBlkSizeY, int32_t _nPel, int32_t _nLevel, int32_t _nMotionFlags, int32_t _nOverlapX, int32_t _nOverlapY, int32_t _xRatioUV, int32_t _yRatioUV) {
		constexpr auto ALIGN_PLANES = 64;
		nPel = _nPel;
		nLogPel = ilog2(nPel);
		nLogScale = _nLevel;
		nScale = iexp2(nLogScale);
		nBlkSizeX = _nBlkSizeX;
		nBlkSizeY = _nBlkSizeY;
		nOverlapX = _nOverlapX;
		nOverlapY = _nOverlapY;
		nBlkX = _nBlkX;
		nBlkY = _nBlkY;
		nBlkCount = nBlkX * nBlkY;
		nMotionFlags = _nMotionFlags;
		xRatioUV = _xRatioUV;
		yRatioUV = _yRatioUV;
		nLogxRatioUV = ilog2(xRatioUV);
		nLogyRatioUV = ilog2(yRatioUV);
		smallestPlane = !!(nMotionFlags & MOTION_SMALLEST_PLANE);
		chroma = !!(nMotionFlags & MOTION_USE_CHROMA_MOTION);
		globalMVPredictor.x = zeroMV.x;
		globalMVPredictor.y = zeroMV.y;
		globalMVPredictor.sad = zeroMV.sad;
		vectors = new VectorStructure[nBlkCount];
		memset(vectors, 0, nBlkCount * sizeof(VectorStructure));
		static SADFunction sads[257][257];
		static LUMAFunction lumas[257][257];
		static COPYFunction blits[257][257];
		static SADFunction satds[257][257];
		sads[2][2] = Sad_C<2, 2>;
		lumas[2][2] = Luma_C<2, 2>;
		blits[2][2] = Copy_C<2, 2>;
		sads[2][4] = Sad_C<2, 4>;
		blits[2][4] = Copy_C<2, 4>;
		sads[4][2] = Sad_C<4, 2>;
		blits[4][2] = Copy_C<4, 2>;
		sads[4][4] = Sad_C<4, 4>;
		lumas[4][4] = Luma_C<4, 4>;
		blits[4][4] = Copy_C<4, 4>;
		satds[4][4] = Satd_C<4, 4>;
		sads[4][8] = Sad_C<4, 8>;
		blits[4][8] = Copy_C<4, 8>;
		sads[8][1] = Sad_C<8, 1>;
		blits[8][1] = Copy_C<8, 1>;
		sads[8][2] = Sad_C<8, 2>;
		blits[8][2] = Copy_C<8, 2>;
		sads[8][4] = Sad_C<8, 4>;
		lumas[8][4] = Luma_C<8, 4>;
		blits[8][4] = Copy_C<8, 4>;
		sads[8][8] = Sad_C<8, 8>;
		lumas[8][8] = Luma_C<8, 8>;
		blits[8][8] = Copy_C<8, 8>;
		satds[8][8] = Satd_C<8, 8>;
		sads[8][16] = Sad_C<8, 16>;
		blits[8][16] = Copy_C<8, 16>;
		sads[16][1] = Sad_C<16, 1>;
		blits[16][1] = Copy_C<16, 1>;
		sads[16][2] = Sad_C<16, 2>;
		lumas[16][2] = Luma_C<16, 2>;
		blits[16][2] = Copy_C<16, 2>;
		sads[16][4] = Sad_C<16, 4>;
		blits[16][4] = Copy_C<16, 4>;
		sads[16][8] = Sad_C<16, 8>;
		lumas[16][8] = Luma_C<16, 8>;
		blits[16][8] = Copy_C<16, 8>;
		sads[16][16] = Sad_C<16, 16>;
		lumas[16][16] = Luma_C<16, 16>;
		blits[16][16] = Copy_C<16, 16>;
		satds[16][16] = Satd_C<16, 16>;
		sads[16][32] = Sad_C<16, 32>;
		blits[16][32] = Copy_C<16, 32>;
		sads[32][8] = Sad_C<32, 8>;
		blits[32][8] = Copy_C<32, 8>;
		sads[32][16] = Sad_C<32, 16>;
		lumas[32][16] = Luma_C<32, 16>;
		blits[32][16] = Copy_C<32, 16>;
		sads[32][32] = Sad_C<32, 32>;
		lumas[32][32] = Luma_C<32, 32>;
		blits[32][32] = Copy_C<32, 32>;
		satds[32][32] = Satd_C<32, 32>;
		sads[32][64] = Sad_C<32, 64>;
		sads[64][16] = Sad_C<64, 16>;
		sads[64][32] = Sad_C<64, 32>;
		sads[64][64] = Sad_C<64, 64>;
		sads[64][128] = Sad_C<64, 128>;
		sads[128][32] = Sad_C<128, 32>;
		sads[128][64] = Sad_C<128, 64>;
		sads[128][128] = Sad_C<128, 128>;
		sads[128][256] = Sad_C<128, 256>;
		sads[256][64] = Sad_C<256, 64>;
		sads[256][128] = Sad_C<256, 128>;
		sads[256][256] = Sad_C<256, 256>;
		lumas[32][64] = Luma_C<32, 64>;
		lumas[64][16] = Luma_C<64, 16>;
		lumas[64][32] = Luma_C<64, 32>;
		lumas[64][64] = Luma_C<64, 64>;
		lumas[64][128] = Luma_C<64, 128>;
		lumas[128][32] = Luma_C<128, 32>;
		lumas[128][64] = Luma_C<128, 64>;
		lumas[128][128] = Luma_C<128, 128>;
		lumas[128][256] = Luma_C<128, 256>;
		lumas[256][64] = Luma_C<256, 64>;
		lumas[256][128] = Luma_C<256, 128>;
		lumas[256][256] = Luma_C<256, 256>;
		blits[32][64] = Copy_C<32, 64>;
		blits[64][16] = Copy_C<64, 16>;
		blits[64][32] = Copy_C<64, 32>;
		blits[64][64] = Copy_C<64, 64>;
		blits[64][128] = Copy_C<64, 128>;
		blits[128][32] = Copy_C<128, 32>;
		blits[128][64] = Copy_C<128, 64>;
		blits[128][128] = Copy_C<128, 128>;
		blits[128][256] = Copy_C<128, 256>;
		blits[256][64] = Copy_C<256, 64>;
		blits[256][128] = Copy_C<256, 128>;
		blits[256][256] = Copy_C<256, 256>;
		satds[64][64] = Satd_C<64, 64>;
		satds[128][128] = Satd_C<128, 128>;
		satds[256][256] = Satd_C<256, 256>;
		SAD = sads[nBlkSizeX][nBlkSizeY];
		LUMA = lumas[nBlkSizeX][nBlkSizeY];
		BLITLUMA = blits[nBlkSizeX][nBlkSizeY];
		SADCHROMA = sads[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
		BLITCHROMA = blits[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
		SATD = satds[nBlkSizeX][nBlkSizeY];
		if (!chroma)
			SADCHROMA = nullptr;
		dctpitch = nBlkSizeX * sizeof(float);
		dctSrc = vs_aligned_malloc<uint8_t>(nBlkSizeY * dctpitch, ALIGN_PLANES);
		dctRef = vs_aligned_malloc<uint8_t>(nBlkSizeY * dctpitch, ALIGN_PLANES);

		nSrcPitch_temp[0] = nBlkSizeX * 4;
		nSrcPitch_temp[1] = nBlkSizeX / xRatioUV * 4;
		nSrcPitch_temp[2] = nSrcPitch_temp[1];

		pSrc_temp[0] = vs_aligned_malloc<uint8_t>(nBlkSizeY * nSrcPitch_temp[0] + 4, ALIGN_PLANES);
		pSrc_temp[1] = vs_aligned_malloc<uint8_t>(nBlkSizeY / yRatioUV * nSrcPitch_temp[1] + 4, ALIGN_PLANES);
		pSrc_temp[2] = vs_aligned_malloc<uint8_t>(nBlkSizeY / yRatioUV * nSrcPitch_temp[2] + 4, ALIGN_PLANES);

		freqSize = 8192 * nPel * 2;
		freqArray = new int32_t[freqSize];
		verybigSAD = 1. * nBlkSizeX * nBlkSizeY;
	}
	~PlaneOfBlocks() {
		delete[] vectors;
		delete[] freqArray;

		vs_aligned_free(dctSrc);
		vs_aligned_free(dctRef);

		vs_aligned_free(pSrc_temp[0]);
		vs_aligned_free(pSrc_temp[1]);
		vs_aligned_free(pSrc_temp[2]);
	}
	auto GetArraySize(int32_t divideMode) {
		int32_t size = 0;
		size += 1;              // mb data size storage
		size += nBlkCount * N_PER_BLOCK;  // vectors, sad, luma src, luma ref, var

		if (nLogScale == 0)
			if (divideMode)
				size += 1 + nBlkCount * N_PER_BLOCK * 4; // reserve space for divided subblocks extra level

		return size;
	}
	void FetchPredictors() {
		// Left (or right) predictor
		if ((blkScanDir == 1 && blkx > 0) || (blkScanDir == -1 && blkx < nBlkX - 1)) predictors[1] = ClipMV(vectors[blkIdx - blkScanDir]);
		else predictors[1] = ClipMV(zeroMVfieldShifted); // v1.11.1 - values instead of pointer

														 // Up predictor
		if (blky > 0) predictors[2] = ClipMV(vectors[blkIdx - nBlkX]);
		else predictors[2] = ClipMV(zeroMVfieldShifted);

		// bottom-right pridictor (from coarse level)
		if ((blky < nBlkY - 1) && ((blkScanDir == 1 && blkx < nBlkX - 1) || (blkScanDir == -1 && blkx > 0)))
			predictors[3] = ClipMV(vectors[blkIdx + nBlkX + blkScanDir]);
		else
			// Up-right predictor
			if ((blky > 0) && ((blkScanDir == 1 && blkx < nBlkX - 1) || (blkScanDir == -1 && blkx > 0)))
				predictors[3] = ClipMV(vectors[blkIdx - nBlkX + blkScanDir]);
			else predictors[3] = ClipMV(zeroMVfieldShifted);

		// Median predictor
		if (blky > 0) // replaced 1 by 0 - Fizick
		{
			predictors[0].x = Median(predictors[1].x, predictors[2].x, predictors[3].x);
			predictors[0].y = Median(predictors[1].y, predictors[2].y, predictors[3].y);
			//      predictors[0].sad = Median(predictors[1].sad, predictors[2].sad, predictors[3].sad);
			// but it is not true median vector (x and y may be mixed) and not its sad ?!
			// we really do not know SAD, here is more safe estimation especially for phaseshift method - v1.6.0
			predictors[0].sad = max(predictors[1].sad, max(predictors[2].sad, predictors[3].sad));
		}
		else {
			//		predictors[0].x = (predictors[1].x + predictors[2].x + predictors[3].x);
			//		predictors[0].y = (predictors[1].y + predictors[2].y + predictors[3].y);
			//      predictors[0].sad = (predictors[1].sad + predictors[2].sad + predictors[3].sad);
			// but for top line we have only left predictor[1] - v1.6.0
			predictors[0].x = predictors[1].x;
			predictors[0].y = predictors[1].y;
			predictors[0].sad = predictors[1].sad;
		}

		// if there are no other planes, predictor is the median
		if (smallestPlane)
			predictor = predictors[0];
		auto coeff = LSAD / (LSAD + predictor.sad / 2.);
		nLambda *= coeff * coeff;
	}
	void DiamondSearch(int32_t length) {
		// The meaning of the directions are the following :
		//		* 1 means right
		//		* 2 means left
		//		* 4 means down
		//		* 8 means up
		// So 1 + 4 means down right, and so on...

		int32_t dx;
		int32_t dy;

		// We begin by making no assumption on which direction to search.
		int32_t direction = 15;

		int32_t lastDirection;

		while (direction > 0)
		{
			dx = bestMV.x;
			dy = bestMV.y;
			lastDirection = direction;
			direction = 0;

			// First, we look the directions that were hinted by the previous step
			// of the algorithm. If we find one, we add it to the set of directions
			// we'll test next
			if (lastDirection & 1) CheckMV2(dx + length, dy, &direction, 1);
			if (lastDirection & 2) CheckMV2(dx - length, dy, &direction, 2);
			if (lastDirection & 4) CheckMV2(dx, dy + length, &direction, 4);
			if (lastDirection & 8) CheckMV2(dx, dy - length, &direction, 8);

			// If one of the directions improves the SAD, we make further tests
			// on the diagonals
			if (direction) {
				lastDirection = direction;
				dx = bestMV.x;
				dy = bestMV.y;

				if (lastDirection & 3)
				{
					CheckMV2(dx, dy + length, &direction, 4);
					CheckMV2(dx, dy - length, &direction, 8);
				}
				else {
					CheckMV2(dx + length, dy, &direction, 1);
					CheckMV2(dx - length, dy, &direction, 2);
				}
			}

			// If not, we do not stop here. We infer from the last direction the
			// diagonals to be checked, because we might be lucky.
			else {
				switch (lastDirection) {
				case 1:
					CheckMV2(dx + length, dy + length, &direction, 1 + 4);
					CheckMV2(dx + length, dy - length, &direction, 1 + 8);
					break;
				case 2:
					CheckMV2(dx - length, dy + length, &direction, 2 + 4);
					CheckMV2(dx - length, dy - length, &direction, 2 + 8);
					break;
				case 4:
					CheckMV2(dx + length, dy + length, &direction, 1 + 4);
					CheckMV2(dx - length, dy + length, &direction, 2 + 4);
					break;
				case 8:
					CheckMV2(dx + length, dy - length, &direction, 1 + 8);
					CheckMV2(dx - length, dy - length, &direction, 2 + 8);
					break;
				case 1 + 4:
					CheckMV2(dx + length, dy + length, &direction, 1 + 4);
					CheckMV2(dx - length, dy + length, &direction, 2 + 4);
					CheckMV2(dx + length, dy - length, &direction, 1 + 8);
					break;
				case 2 + 4:
					CheckMV2(dx + length, dy + length, &direction, 1 + 4);
					CheckMV2(dx - length, dy + length, &direction, 2 + 4);
					CheckMV2(dx - length, dy - length, &direction, 2 + 8);
					break;
				case 1 + 8:
					CheckMV2(dx + length, dy + length, &direction, 1 + 4);
					CheckMV2(dx - length, dy - length, &direction, 2 + 8);
					CheckMV2(dx + length, dy - length, &direction, 1 + 8);
					break;
				case 2 + 8:
					CheckMV2(dx - length, dy - length, &direction, 2 + 8);
					CheckMV2(dx - length, dy + length, &direction, 2 + 4);
					CheckMV2(dx + length, dy - length, &direction, 1 + 8);
					break;
				default:
					// Even the default case may happen, in the first step of the
					// algorithm for example.
					CheckMV2(dx + length, dy + length, &direction, 1 + 4);
					CheckMV2(dx - length, dy + length, &direction, 2 + 4);
					CheckMV2(dx + length, dy - length, &direction, 1 + 8);
					CheckMV2(dx - length, dy - length, &direction, 2 + 8);
					break;
				}
			}
		}
	}
	void NStepSearch(int32_t stp) {
		int32_t dx, dy;
		int32_t length = stp;
		while (length > 0)
		{
			dx = bestMV.x;
			dy = bestMV.y;

			CheckMV(dx + length, dy + length);
			CheckMV(dx + length, dy);
			CheckMV(dx + length, dy - length);
			CheckMV(dx, dy - length);
			CheckMV(dx, dy + length);
			CheckMV(dx - length, dy + length);
			CheckMV(dx - length, dy);
			CheckMV(dx - length, dy - length);

			length--;
		}
	}
	void OneTimeSearch(int32_t length) {
		int32_t direction = 0;
		int32_t dx = bestMV.x;
		int32_t dy = bestMV.y;

		CheckMV2(dx - length, dy, &direction, 2);
		CheckMV2(dx + length, dy, &direction, 1);

		if (direction == 1)
		{
			while (direction)
			{
				direction = 0;
				dx += length;
				CheckMV2(dx + length, dy, &direction, 1);
			}
		}
		else if (direction == 2)
		{
			while (direction)
			{
				direction = 0;
				dx -= length;
				CheckMV2(dx - length, dy, &direction, 1);
			}
		}

		CheckMV2(dx, dy - length, &direction, 2);
		CheckMV2(dx, dy + length, &direction, 1);

		if (direction == 1)
		{
			while (direction)
			{
				direction = 0;
				dy += length;
				CheckMV2(dx, dy + length, &direction, 1);
			}
		}
		else if (direction == 2)
		{
			while (direction)
			{
				direction = 0;
				dy -= length;
				CheckMV2(dx, dy - length, &direction, 1);
			}
		}
	}
	void PseudoEPZSearch() {

		FetchPredictors();

		double sad;
		double saduv;

		if (dctmode != 0) // DCT method (luma only - currently use normal spatial SAD chroma)
		{
			// make dct of source block
			if (dctmode <= 4) //don't do the slow dct conversion if SATD used
				DCT->DCTBytes2D(pSrc[0], nSrcPitch[0], dctSrc, dctpitch);
		}
		if (dctmode >= 3) // most use it and it should be fast anyway //if (dctmode == 3 || dctmode == 4) // check it
			srcLuma = LUMA(pSrc[0], nSrcPitch[0]);


		// We treat zero alone
		// Do we bias zero with not taking into account distorsion ?
		bestMV.x = zeroMVfieldShifted.x;
		bestMV.y = zeroMVfieldShifted.y;
		saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(0, 0), nRefPitch[1])
			+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(0, 0), nRefPitch[2]) : 0.f;
		sad = LumaSAD(GetRefBlock(0, zeroMVfieldShifted.y));
		sad += saduv;
		bestMV.sad = sad;
		nMinCost = sad + ((penaltyZero * sad) / 256); // v.1.11.0.2

		VectorStructure bestMVMany[8];
		double nMinCostMany[8];

		if (tryMany)
		{
			//  refine around zero
			Refine();
			bestMVMany[0] = bestMV;   // save bestMV
			nMinCostMany[0] = nMinCost;
		}

		// Global MV predictor  - added by Fizick
		globalMVPredictor = ClipMV(globalMVPredictor);
		saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(globalMVPredictor.x, globalMVPredictor.y), nRefPitch[1])
			+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(globalMVPredictor.x, globalMVPredictor.y), nRefPitch[2]) : 0.f;
		sad = LumaSAD(GetRefBlock(globalMVPredictor.x, globalMVPredictor.y));
		sad += saduv;
		double cost = sad + ((pglobal * sad) / 256.);

		if (cost < nMinCost || tryMany)
		{
			bestMV.x = globalMVPredictor.x;
			bestMV.y = globalMVPredictor.y;
			bestMV.sad = sad;
			nMinCost = cost;
		}
		if (tryMany)
		{
			// refine around global
			Refine();    // reset bestMV
			bestMVMany[1] = bestMV;    // save bestMV
			nMinCostMany[1] = nMinCost;
		}
		saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(predictor.x, predictor.y), nRefPitch[1])
			+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(predictor.x, predictor.y), nRefPitch[2]) : 0.f;
		sad = LumaSAD(GetRefBlock(predictor.x, predictor.y));
		sad += saduv;
		cost = sad;

		if (cost < nMinCost || tryMany)
		{
			bestMV.x = predictor.x;
			bestMV.y = predictor.y;
			bestMV.sad = sad;
			nMinCost = cost;
		}
		if (tryMany)
		{
			// refine around predictor
			Refine();    // reset bestMV
			bestMVMany[2] = bestMV;   // save bestMV
			nMinCostMany[2] = nMinCost;
		}

		// then all the other predictors
		int32_t npred = (temporal) ? 5 : 4;
		constexpr auto epsilon = 1e-5;

		for (int32_t i = 0; i < npred; i++)
		{
			if (tryMany)
				nMinCost = verybigSAD + epsilon;
			CheckMV0(predictors[i].x, predictors[i].y);
			if (tryMany)
			{
				// refine around predictor
				Refine();    // reset bestMV
				bestMVMany[i + 3] = bestMV;    // save bestMV
				nMinCostMany[i + 3] = nMinCost;
			}
		}


		if (tryMany) { // select best of multi best
			nMinCost = verybigSAD + epsilon;
			for (int32_t i = 0; i < npred + 3; i++)
			{
				if (nMinCostMany[i] < nMinCost)
				{
					bestMV = bestMVMany[i];
					nMinCost = nMinCostMany[i];
				}
			}
		}
		else
		{
			// then, we refine, according to the search type
			Refine();
		}

		auto foundSAD = bestMV.sad;

		constexpr auto BADCOUNT_LIMIT = 16;

		if (blkIdx > 1 && foundSAD > (badSAD + badSAD * badcount / BADCOUNT_LIMIT)) // bad vector, try wide search
		{// with some soft limit (BADCOUNT_LIMIT) of bad cured vectors (time consumed)
			badcount++;

			if (badrange > 0) // UMH
			{


				{
					// rathe good is not found, lets try around zero
					UMHSearch(badrange * nPel, 0, 0);
				}

			}
			else if (badrange < 0) // ESA
			{

				for (int32_t i = 1; i < -badrange * nPel; i += nPel)// at radius
				{
					ExpandingSearch(i, nPel, 0, 0);
					if (bestMV.sad < foundSAD / 4) break; // stop search if rathe good is found
				}
			}

			int32_t mvx = bestMV.x; // refine in small area
			int32_t mvy = bestMV.y;
			for (int32_t i = 1; i < nPel; i++)// small radius
			{
				ExpandingSearch(i, 1, mvx, mvy);
			}
		}


		// we store the result
		vectors[blkIdx].x = bestMV.x;
		vectors[blkIdx].y = bestMV.y;
		vectors[blkIdx].sad = bestMV.sad;

		planeSAD += bestMV.sad;

	}
	void ExpandingSearch(int32_t r, int32_t s, int32_t mvx, int32_t mvy) {
		int32_t i, j;

		// sides of square without corners
		for (i = -r + s; i < r; i += s) // without corners! - v2.1
		{
			CheckMV(mvx + i, mvy - r);
			CheckMV(mvx + i, mvy + r);
		}

		for (j = -r + s; j < r; j += s)
		{
			CheckMV(mvx - r, mvy + j);
			CheckMV(mvx + r, mvy + j);
		}

		// then corners - they are more far from cenrer
		CheckMV(mvx - r, mvy - r);
		CheckMV(mvx - r, mvy + r);
		CheckMV(mvx + r, mvy - r);
		CheckMV(mvx + r, mvy + r);
	}
	void Hex2Search(int32_t i_me_range) {
		auto zip = [](auto x, auto y) {
			return std::array{ x, y };
		};
		auto mod6m1 = std::array{ 5,0,1,2,3,4,5,0 };
		auto hex2 = std::array{ zip(-1,-2),zip(-2,0),zip(-1,2),zip(1,2),zip(2,0),zip(1,-2),zip(-1,-2),zip(-2,0) };
		int32_t dir = -2;
		int32_t bmx = bestMV.x;
		int32_t bmy = bestMV.y;

		if (i_me_range > 1)
		{
			/* hexagon */
			//        COST_MV_X3_DIR( -2,0, -1, 2,  1, 2, costs   );
			//        COST_MV_X3_DIR(  2,0,  1,-2, -1,-2, costs+3 );
			//        COPY2_IF_LT( bcost, costs[0], dir, 0 );
			//        COPY2_IF_LT( bcost, costs[1], dir, 1 );
			//        COPY2_IF_LT( bcost, costs[2], dir, 2 );
			//        COPY2_IF_LT( bcost, costs[3], dir, 3 );
			//        COPY2_IF_LT( bcost, costs[4], dir, 4 );
			//        COPY2_IF_LT( bcost, costs[5], dir, 5 );
			CheckMVdir(bmx - 2, bmy, &dir, 0);
			CheckMVdir(bmx - 1, bmy + 2, &dir, 1);
			CheckMVdir(bmx + 1, bmy + 2, &dir, 2);
			CheckMVdir(bmx + 2, bmy, &dir, 3);
			CheckMVdir(bmx + 1, bmy - 2, &dir, 4);
			CheckMVdir(bmx - 1, bmy - 2, &dir, 5);


			if (dir != -2)
			{
				bmx += hex2[dir + 1][0];
				bmy += hex2[dir + 1][1];
				/* half hexagon, not overlapping the previous iteration */
				for (int32_t i = 1; i < i_me_range / 2 && IsVectorOK(bmx, bmy); i++)
				{
					const int32_t odir = mod6m1[dir + 1];
					//                COST_MV_X3_DIR( hex2[odir+0][0], hex2[odir+0][1],
					//                                hex2[odir+1][0], hex2[odir+1][1],
					//                                hex2[odir+2][0], hex2[odir+2][1],
					//                                costs );

					dir = -2;
					//                COPY2_IF_LT( bcost, costs[0], dir, odir-1 );
					//                COPY2_IF_LT( bcost, costs[1], dir, odir   );
					//                COPY2_IF_LT( bcost, costs[2], dir, odir+1 );

					CheckMVdir(bmx + hex2[odir + 0][0], bmy + hex2[odir + 0][1], &dir, odir - 1);
					CheckMVdir(bmx + hex2[odir + 1][0], bmy + hex2[odir + 1][1], &dir, odir);
					CheckMVdir(bmx + hex2[odir + 2][0], bmy + hex2[odir + 2][1], &dir, odir + 1);
					if (dir == -2)
						break;
					bmx += hex2[dir + 1][0];
					bmy += hex2[dir + 1][1];
				}
			}

			bestMV.x = bmx;
			bestMV.y = bmy;
		}
		/* square refine */
		//        omx = bmx; omy = bmy;
		//        COST_MV_X4(  0,-1,  0,1, -1,0, 1,0 );
		//        COST_MV_X4( -1,-1, -1,1, 1,-1, 1,1 );
		ExpandingSearch(1, 1, bmx, bmy);

	}
	void CrossSearch(int32_t start, int32_t x_max, int32_t y_max, int32_t mvx, int32_t mvy) {

		for (int32_t i = start; i < x_max; i += 2)
		{
			CheckMV(mvx - i, mvy);
			CheckMV(mvx + i, mvy);
		}

		for (int32_t j = start; j < y_max; j += 2)
		{
			CheckMV(mvx, mvy + j);
			CheckMV(mvx, mvy + j);
		}
	}
	void UMHSearch(int32_t i_me_range, int32_t omx, int32_t omy) {
		// Uneven-cross Multi-Hexagon-grid Search (see x264)
		/* hexagon grid */

		//            int32_t omx = bestMV.x;
		//            int32_t omy = bestMV.y;
		// my mod: do not shift the center after Cross
		CrossSearch(1, i_me_range, i_me_range, omx, omy);


		int32_t i = 1;
		do
		{
			static const int32_t hex4[16][2] = {
				{ -4, 2 },{ -4, 1 },{ -4, 0 },{ -4,-1 },{ -4,-2 },
				{ 4,-2 },{ 4,-1 },{ 4, 0 },{ 4, 1 },{ 4, 2 },
				{ 2, 3 },{ 0, 4 },{ -2, 3 },
				{ -2,-3 },{ 0,-4 },{ 2,-3 },
			};

			for (int32_t j = 0; j < 16; j++)
			{
				int32_t mx = omx + hex4[j][0] * i;
				int32_t my = omy + hex4[j][1] * i;
				CheckMV(mx, my);
			}
		} while (++i <= i_me_range / 4);

		//            if( bmy <= mv_y_max )
		//                goto me_hex2;
		Hex2Search(i_me_range);

	}
	void SearchMVs(MVFrame* _pSrcFrame, MVFrame* _pRefFrame,
		SearchType st, int32_t stp, double lambda, double lsad, int32_t pnew,
		int32_t plevel, int32_t* out, VectorStructure* globalMVec,
		int32_t* outfilebuf, int32_t fieldShift, DCTClass* _DCT, double* pmeanLumaChange,
		int32_t divideExtra, int32_t _pzero, int32_t _pglobal, double _badSAD, int32_t _badrange, bool meander, int32_t* vecPrev, bool _tryMany) {
		DCT = _DCT;
		if (DCT == 0)
			dctmode = 0;
		else
			dctmode = DCT->dctmode;
		dctweight16 = min(16, std::abs(*pmeanLumaChange) / (nBlkSizeX * nBlkSizeY)); //equal dct and spatial weights for meanLumaChange=8 (empirical)
		badSAD = _badSAD;
		badrange = _badrange;
		zeroMVfieldShifted.x = 0;
		zeroMVfieldShifted.y = fieldShift;
		zeroMVfieldShifted.sad = 0.f;
		globalMVPredictor.x = nPel * globalMVec->x;// v1.8.2
		globalMVPredictor.y = nPel * globalMVec->y + fieldShift;
		globalMVPredictor.sad = globalMVec->sad;

		// write the plane's header
		WriteHeaderToArray(out);

		int32_t* pBlkData = out + 1;
		temporal = (vecPrev) ? true : false;
		if (vecPrev) vecPrev += 1; // same as BlkData

		pSrcFrame = _pSrcFrame;
		pRefFrame = _pRefFrame;


		y[0] = pSrcFrame->GetPlane(YPLANE)->GetVPadding();

		if (pSrcFrame->GetMode() & UPLANE)
		{
			y[1] = pSrcFrame->GetPlane(UPLANE)->GetVPadding();
		}
		if (pSrcFrame->GetMode() & VPLANE)
		{
			y[2] = pSrcFrame->GetPlane(VPLANE)->GetVPadding();
		}

		nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
		if (chroma)
		{
			nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
			nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
		}

		nRefPitch[0] = pRefFrame->GetPlane(YPLANE)->GetPitch();
		if (chroma)
		{
			nRefPitch[1] = pRefFrame->GetPlane(UPLANE)->GetPitch();
			nRefPitch[2] = pRefFrame->GetPlane(VPLANE)->GetPitch();
		}

		searchType = st;//( nLogScale == 0 ) ? st : EXHAUSTIVE;
		nSearchParam = stp;//*nPel; // v1.8.2 - redesigned in v1.8.5

		double nLambdaLevel = lambda / (nPel * nPel);
		if (plevel == 1)
			nLambdaLevel = nLambdaLevel * nScale;// scale lambda - Fizick
		else if (plevel == 2)
			nLambdaLevel = nLambdaLevel * nScale * nScale;

		penaltyZero = _pzero;
		pglobal = _pglobal;
		planeSAD = 0.0;
		badcount = 0;
		tryMany = _tryMany;
		sumLumaChange = 0.;
		// Functions using double must not be used here

		for (blky = 0; blky < nBlkY; blky++)
		{
			blkScanDir = (blky % 2 == 0 || meander == false) ? 1 : -1;
			// meander (alternate) scan blocks (even row left to right, odd row right to left)
			int32_t blkxStart = (blky % 2 == 0 || meander == false) ? 0 : nBlkX - 1;
			if (blkScanDir == 1) // start with leftmost block
			{
				x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding();
				if (chroma)
				{
					x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding();
					x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding();
				}
			}
			else // start with rightmost block, but it is already set at prev row
			{
				x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding() + (nBlkSizeX - nOverlapX) * (nBlkX - 1);
				if (chroma)
				{
					x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding() + ((nBlkSizeX - nOverlapX) / xRatioUV) * (nBlkX - 1);
					x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding() + ((nBlkSizeX - nOverlapX) / xRatioUV) * (nBlkX - 1);
				}
			}
			for (int32_t iblkx = 0; iblkx < nBlkX; iblkx++)
			{
				blkx = blkxStart + iblkx * blkScanDir;
				blkIdx = blky * nBlkX + blkx;
				iter = 0;

				pSrc[0] = pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(x[0], y[0]);
				if (chroma)
				{
					pSrc[1] = pSrcFrame->GetPlane(UPLANE)->GetAbsolutePelPointer(x[1], y[1]);
					pSrc[2] = pSrcFrame->GetPlane(VPLANE)->GetAbsolutePelPointer(x[2], y[2]);
				}

				nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
				//create aligned copy
				BLITLUMA(pSrc_temp[0], nSrcPitch_temp[0], pSrc[0], nSrcPitch[0]);
				//set the to the aligned copy
				pSrc[0] = pSrc_temp[0];
				nSrcPitch[0] = nSrcPitch_temp[0];
				if (chroma)
				{
					nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
					nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
					BLITCHROMA(pSrc_temp[1], nSrcPitch_temp[1], pSrc[1], nSrcPitch[1]);
					BLITCHROMA(pSrc_temp[2], nSrcPitch_temp[2], pSrc[2], nSrcPitch[2]);
					pSrc[1] = pSrc_temp[1];
					pSrc[2] = pSrc_temp[2];
					nSrcPitch[1] = nSrcPitch_temp[1];
					nSrcPitch[2] = nSrcPitch_temp[2];
				}

				if (blky == 0)
					nLambda = 0.;
				else
					nLambda = nLambdaLevel;

				penaltyNew = pnew; // penalty for new vector
				LSAD = lsad;    // SAD limit for lambda using
								// may be they must be scaled by nPel ?

								// decreased padding of coarse levels
				int32_t nHPaddingScaled = pSrcFrame->GetPlane(YPLANE)->GetHPadding() >> nLogScale;
				int32_t nVPaddingScaled = pSrcFrame->GetPlane(YPLANE)->GetVPadding() >> nLogScale;
				/* computes search boundaries */
				nDxMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedWidth() - x[0] - nBlkSizeX - pSrcFrame->GetPlane(YPLANE)->GetHPadding() + nHPaddingScaled);
				nDyMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedHeight() - y[0] - nBlkSizeY - pSrcFrame->GetPlane(YPLANE)->GetVPadding() + nVPaddingScaled);
				nDxMin = -nPel * (x[0] - pSrcFrame->GetPlane(YPLANE)->GetHPadding() + nHPaddingScaled);
				nDyMin = -nPel * (y[0] - pSrcFrame->GetPlane(YPLANE)->GetVPadding() + nVPaddingScaled);

				/* search the mv */
				predictor = ClipMV(vectors[blkIdx]);
				if (temporal)
					predictors[4] = ClipMV(reinterpret_cast<VectorStructure&>(vecPrev[blkIdx * N_PER_BLOCK])); // temporal predictor
				else
					predictors[4] = ClipMV(zeroMV);

				PseudoEPZSearch();


				/* write the results */

				auto& BlockData = reinterpret_cast<VectorStructure&>(pBlkData[blkx * N_PER_BLOCK]);
				BlockData = bestMV;


				if (smallestPlane)
					sumLumaChange += LUMA(GetRefBlock(0, 0), nRefPitch[0]) - LUMA(pSrc[0], nSrcPitch[0]);

				/* increment indexes & pointers */
				if (iblkx < nBlkX - 1)
				{
					x[0] += (nBlkSizeX - nOverlapX) * blkScanDir;
					if (pSrcFrame->GetMode() & UPLANE)
						x[1] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
					if (pSrcFrame->GetMode() & VPLANE)
						x[2] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
				}
			}
			pBlkData += nBlkX * N_PER_BLOCK;

			y[0] += (nBlkSizeY - nOverlapY);
			if (pSrcFrame->GetMode() & UPLANE)
				y[1] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV);
			if (pSrcFrame->GetMode() & VPLANE)
				y[2] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV);
		}
		if (smallestPlane)
			*pmeanLumaChange = sumLumaChange / nBlkCount; // for all finer planes

	}
	void InterpolatePrediction(const PlaneOfBlocks& pob) {
		int32_t normFactor = 3 - nLogPel + pob.nLogPel;
		int32_t mulFactor = (normFactor < 0) ? -normFactor : 0;
		normFactor = (normFactor < 0) ? 0 : normFactor;
		int32_t normov = (nBlkSizeX - nOverlapX) * (nBlkSizeY - nOverlapY);
		int32_t aoddx = (nBlkSizeX * 3 - nOverlapX * 2);
		int32_t aevenx = (nBlkSizeX * 3 - nOverlapX * 4);
		int32_t aoddy = (nBlkSizeY * 3 - nOverlapY * 2);
		int32_t aeveny = (nBlkSizeY * 3 - nOverlapY * 4);
		// note: overlapping is still (v2.5.7) not proceerly
		for (int32_t l = 0, index = 0; l < nBlkY; l++)
		{
			for (int32_t k = 0; k < nBlkX; k++, index++)
			{
				VectorStructure v1, v2, v3, v4;
				int32_t i = k;
				int32_t j = l;
				if (i >= 2 * pob.nBlkX) i = 2 * pob.nBlkX - 1;
				if (j >= 2 * pob.nBlkY) j = 2 * pob.nBlkY - 1;
				int32_t offy = -1 + 2 * (j % 2);
				int32_t offx = -1 + 2 * (i % 2);

				if ((i == 0) || (i >= 2 * pob.nBlkX - 1))
				{
					if ((j == 0) || (j >= 2 * pob.nBlkY - 1))
					{
						v1 = v2 = v3 = v4 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
					}
					else
					{
						v1 = v2 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
						v3 = v4 = pob.vectors[i / 2 + (j / 2 + offy) * pob.nBlkX];
					}
				}
				else if ((j == 0) || (j >= 2 * pob.nBlkY - 1))
				{
					v1 = v2 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
					v3 = v4 = pob.vectors[i / 2 + offx + (j / 2) * pob.nBlkX];
				}
				else
				{
					v1 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
					v2 = pob.vectors[i / 2 + offx + (j / 2) * pob.nBlkX];
					v3 = pob.vectors[i / 2 + (j / 2 + offy) * pob.nBlkX];
					v4 = pob.vectors[i / 2 + offx + (j / 2 + offy) * pob.nBlkX];
				}

				double temp_sad;

				if (nOverlapX == 0 && nOverlapY == 0)
				{
					vectors[index].x = 9 * v1.x + 3 * v2.x + 3 * v3.x + v4.x;
					vectors[index].y = 9 * v1.y + 3 * v2.y + 3 * v3.y + v4.y;
					temp_sad = 9 * v1.sad + 3 * v2.sad + 3 * v3.sad + v4.sad;
				}
				else if (nOverlapX <= (nBlkSizeX >> 1) && nOverlapY <= (nBlkSizeY >> 1)) // corrected in v1.4.11
				{
					int32_t	ax1 = (offx > 0) ? aoddx : aevenx;
					int32_t ax2 = (nBlkSizeX - nOverlapX) * 4 - ax1;
					int32_t ay1 = (offy > 0) ? aoddy : aeveny;
					int32_t ay2 = (nBlkSizeY - nOverlapY) * 4 - ay1;
					// 64 bit so that the multiplications by the SADs don't overflow with 16 bit input.
					int64_t a11 = ax1 * ay1, a12 = ax1 * ay2, a21 = ax2 * ay1, a22 = ax2 * ay2;
					vectors[index].x = int32_t((a11 * v1.x + a21 * v2.x + a12 * v3.x + a22 * v4.x) / normov);
					vectors[index].y = int32_t((a11 * v1.y + a21 * v2.y + a12 * v3.y + a22 * v4.y) / normov);
					temp_sad = (a11 * v1.sad + a21 * v2.sad + a12 * v3.sad + a22 * v4.sad) / normov;
				}
				else // large overlap. Weights are not quite correct but let it be
				{
					// Dead branch. The overlap is no longer allowed to be more than half the block size.
					vectors[index].x = (v1.x + v2.x + v3.x + v4.x) << 2;
					vectors[index].y = (v1.y + v2.y + v3.y + v4.y) << 2;
					temp_sad = (v1.sad + v2.sad + v3.sad + v4.sad) * 4;
				}
				vectors[index].x = vectors[index].x ? vectors[index].x / abs(vectors[index].x) * ((abs(vectors[index].x) >> normFactor) << mulFactor) : 0;
				vectors[index].y = vectors[index].y ? vectors[index].y / abs(vectors[index].y) * ((abs(vectors[index].y) >> normFactor) << mulFactor) : 0;
				vectors[index].sad = temp_sad / 16;
			}
		}
	}
	void WriteHeaderToArray(int32_t* array) {
		array[0] = nBlkCount * N_PER_BLOCK + 1;
	}
	auto WriteDefaultToArray(int32_t* array, int32_t divideMode) {
		array[0] = nBlkCount * N_PER_BLOCK + 1;
		for (auto i : Range{ 0, nBlkCount * N_PER_BLOCK, N_PER_BLOCK }) {
			auto& BlockData = reinterpret_cast<VectorStructure&>(array[i + 1]);
			BlockData.x = 0;
			BlockData.y = 0;
			BlockData.sad = verybigSAD;
		}
		if (nLogScale == 0) {
			array += array[0];
			if (divideMode) { // reserve space for divided subblocks extra level
				array[0] = nBlkCount * N_PER_BLOCK * 4 + 1; // 4 subblocks
				for (auto i : Range{ 0, nBlkCount * 4 * N_PER_BLOCK, N_PER_BLOCK }) {
					auto& BlockData = reinterpret_cast<VectorStructure&>(array[i + 1]);
					BlockData.x = 0;
					BlockData.y = 0;
					BlockData.sad = verybigSAD;
				}
				array += array[0];
			}
		}
		return GetArraySize(divideMode);
	}
	void EstimateGlobalMVDoubled(VectorStructure* globalMVec) {
		// estimate global motion from current plane vectors data for using on next plane - added by Fizick
		// on input globalMVec is prev estimation
		// on output globalMVec is doubled for next scale plane using

		// use very simple but robust method
		// more advanced method (like MVDepan) can be implemented later

		// find most frequent x
		memset(&freqArray[0], 0, freqSize * sizeof(int32_t)); // reset
		int32_t indmin = freqSize - 1;
		int32_t indmax = 0;
		for (int32_t i = 0; i < nBlkCount; i++)
		{
			int32_t ind = (freqSize >> 1) + vectors[i].x;
			if (ind >= 0 && ind < freqSize)
			{
				freqArray[ind] += 1;
				if (ind > indmax)
					indmax = ind;
				if (ind < indmin)
					indmin = ind;
			}
		}
		int32_t count = freqArray[indmin];
		int32_t index = indmin;
		for (int32_t i = indmin + 1; i <= indmax; i++)
		{
			if (freqArray[i] > count)
			{
				count = freqArray[i];
				index = i;
			}
		}
		int32_t medianx = (index - (freqSize >> 1)); // most frequent value

												 // find most frequent y
		memset(&freqArray[0], 0, freqSize * sizeof(int32_t)); // reset
		indmin = freqSize - 1;
		indmax = 0;
		for (int32_t i = 0; i < nBlkCount; i++)
		{
			int32_t ind = (freqSize >> 1) + vectors[i].y;
			if (ind >= 0 && ind < freqSize)
			{
				freqArray[ind] += 1;
				if (ind > indmax)
					indmax = ind;
				if (ind < indmin)
					indmin = ind;
			}
		}
		count = freqArray[indmin];
		index = indmin;
		for (int32_t i = indmin + 1; i <= indmax; i++)
		{
			if (freqArray[i] > count)
			{
				count = freqArray[i];
				index = i;
			}
		}
		int32_t mediany = (index - (freqSize >> 1));
		int32_t meanvx = 0;
		int32_t meanvy = 0;
		int32_t num = 0;
		for (int32_t i = 0; i < nBlkCount; i++)
		{
			if (abs(vectors[i].x - medianx) < 6 && abs(vectors[i].y - mediany) < 6)
			{
				meanvx += vectors[i].x;
				meanvy += vectors[i].y;
				num += 1;
			}
		}
		if (num > 0)
		{
			globalMVec->x = 2 * meanvx / num;
			globalMVec->y = 2 * meanvy / num;
		}
		else
		{
			globalMVec->x = 2 * medianx;
			globalMVec->y = 2 * mediany;
		}
	}
	inline int32_t GetnBlkX() { return nBlkX; }
	inline int32_t GetnBlkY() { return nBlkY; }
	void RecalculateMVs(MVClipBalls& mvClip, MVFrame* _pSrcFrame, MVFrame* _pRefFrame,
		SearchType st, int32_t stp, double lambda, int32_t pnew, int32_t* out,
		int32_t* outfilebuf, int32_t fieldShift, double thSAD, DCTClass* _DCT, int32_t divideExtra, int32_t smooth, bool meander) {
		DCT = _DCT;
		if (DCT == 0)
			dctmode = 0;
		else
			dctmode = DCT->dctmode;
		dctweight16 = 8.;//min(16,std::abs(*pmeanLumaChange)/(nBlkSizeX*nBlkSizeY)); //equal dct and spatial weights for meanLumaChange=8 (empirical)
		zeroMVfieldShifted.x = 0;
		zeroMVfieldShifted.y = fieldShift;
		globalMVPredictor.x = 0;//nPel*globalMVec->x;// there is no global
		globalMVPredictor.y = fieldShift;//nPel*globalMVec->y + fieldShift;
		globalMVPredictor.sad = 9999999.f;//globalMVec->sad;

												   // write the plane's header
		WriteHeaderToArray(out);

		int32_t* pBlkData = out + 1;

		pSrcFrame = _pSrcFrame;
		pRefFrame = _pRefFrame;

		x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding();
		y[0] = pSrcFrame->GetPlane(YPLANE)->GetVPadding();
		if (chroma)
		{
			x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding();
			x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding();
			y[1] = pSrcFrame->GetPlane(UPLANE)->GetVPadding();
			y[2] = pSrcFrame->GetPlane(VPLANE)->GetVPadding();
		}

		nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
		if (chroma)
		{
			nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
			nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
		}

		nRefPitch[0] = pRefFrame->GetPlane(YPLANE)->GetPitch();
		if (chroma)
		{
			nRefPitch[1] = pRefFrame->GetPlane(UPLANE)->GetPitch();
			nRefPitch[2] = pRefFrame->GetPlane(VPLANE)->GetPitch();
		}

		searchType = st;
		nSearchParam = stp;//*nPel; // v1.8.2 - redesigned in v1.8.5

		double nLambdaLevel = lambda / (nPel * nPel);

		// get old vectors plane
		const FakePlaneOfBlocks& plane = mvClip[0];
		int32_t nBlkXold = plane.GetReducedWidth();
		int32_t nBlkYold = plane.GetReducedHeight();
		int32_t nBlkSizeXold = plane.GetBlockSizeX();
		int32_t nBlkSizeYold = plane.GetBlockSizeY();
		int32_t nOverlapXold = plane.GetOverlapX();
		int32_t nOverlapYold = plane.GetOverlapY();
		int32_t nStepXold = nBlkSizeXold - nOverlapXold;
		int32_t nStepYold = nBlkSizeYold - nOverlapYold;
		int32_t nPelold = plane.GetPel();
		int32_t nLogPelold = ilog2(nPelold);

		// Functions using double must not be used here
		for (blky = 0; blky < nBlkY; blky++)
		{
			blkScanDir = (blky % 2 == 0 || meander == false) ? 1 : -1;
			// meander (alternate) scan blocks (even row left to right, odd row right to left)
			int32_t blkxStart = (blky % 2 == 0 || meander == false) ? 0 : nBlkX - 1;
			if (blkScanDir == 1) // start with leftmost block
			{
				x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding();
				if (chroma)
				{
					x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding();
					x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding();
				}
			}
			else // start with rightmost block, but it is already set at prev row
			{
				x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding() + (nBlkSizeX - nOverlapX) * (nBlkX - 1);
				if (chroma)
				{
					x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding() + ((nBlkSizeX - nOverlapX) / xRatioUV) * (nBlkX - 1);
					x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding() + ((nBlkSizeX - nOverlapX) / xRatioUV) * (nBlkX - 1);
				}
			}
			for (int32_t iblkx = 0; iblkx < nBlkX; iblkx++)
			{
				blkx = blkxStart + iblkx * blkScanDir;
				blkIdx = blky * nBlkX + blkx;

				pSrc[0] = pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(x[0], y[0]);
				if (chroma)
				{
					pSrc[1] = pSrcFrame->GetPlane(UPLANE)->GetAbsolutePelPointer(x[1], y[1]);
					pSrc[2] = pSrcFrame->GetPlane(VPLANE)->GetAbsolutePelPointer(x[2], y[2]);
				}

				nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
				//create aligned copy
				BLITLUMA(pSrc_temp[0], nSrcPitch_temp[0], pSrc[0], nSrcPitch[0]);
				//set the to the aligned copy
				pSrc[0] = pSrc_temp[0];
				nSrcPitch[0] = nSrcPitch_temp[0];
				if (chroma)
				{
					nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
					nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
					BLITCHROMA(pSrc_temp[1], nSrcPitch_temp[1], pSrc[1], nSrcPitch[1]);
					BLITCHROMA(pSrc_temp[2], nSrcPitch_temp[2], pSrc[2], nSrcPitch[2]);
					pSrc[1] = pSrc_temp[1];
					pSrc[2] = pSrc_temp[2];
					nSrcPitch[1] = nSrcPitch_temp[1];
					nSrcPitch[2] = nSrcPitch_temp[2];
				}


				if (blky == 0)
					nLambda = 0.;
				else
					nLambda = nLambdaLevel;

				penaltyNew = pnew; // penalty for new vector
								   // may be they must be scaled by nPel ?

								   /* computes search boundaries */
				nDxMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedWidth() - x[0] - nBlkSizeX);
				nDyMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedHeight() - y[0] - nBlkSizeY);
				nDxMin = -nPel * x[0];
				nDyMin = -nPel * y[0];

				// get and interplolate old vectors
				int32_t centerX = nBlkSizeX / 2 + (nBlkSizeX - nOverlapX) * blkx; // center of new block
				int32_t blkxold = (centerX - nBlkSizeXold / 2) / nStepXold; // centerXold less or equal to new
				int32_t centerY = nBlkSizeY / 2 + (nBlkSizeY - nOverlapY) * blky;
				int32_t blkyold = (centerY - nBlkSizeYold / 2) / nStepYold;

				int32_t deltaX = max(0, centerX - (nBlkSizeXold / 2 + nStepXold * blkxold)); // distance from old to new
				int32_t deltaY = max(0, centerY - (nBlkSizeYold / 2 + nStepYold * blkyold));

				int32_t blkxold1 = min(nBlkXold - 1, max(0, blkxold));
				int32_t blkxold2 = min(nBlkXold - 1, max(0, blkxold + 1));
				int32_t blkyold1 = min(nBlkYold - 1, max(0, blkyold));
				int32_t blkyold2 = min(nBlkYold - 1, max(0, blkyold + 1));

				auto vectorOld = VectorStructure{}; // interpolated or nearest

				if (smooth == 1) // interpolate
				{

					auto vectorOld1 = mvClip[0][blkxold1 + blkyold1 * nBlkXold].GetMV(); // 4 old nearest vectors (may coinside)
					auto vectorOld2 = mvClip[0][blkxold2 + blkyold1 * nBlkXold].GetMV();
					auto vectorOld3 = mvClip[0][blkxold1 + blkyold2 * nBlkXold].GetMV();
					auto vectorOld4 = mvClip[0][blkxold2 + blkyold2 * nBlkXold].GetMV();

					// interpolate
					int32_t vector1_x = vectorOld1.x * nStepXold + deltaX * (vectorOld2.x - vectorOld1.x); // scaled by nStepXold to skip slow division
					int32_t vector1_y = vectorOld1.y * nStepXold + deltaX * (vectorOld2.y - vectorOld1.y);
					auto vector1_sad = vectorOld1.sad * nStepXold + deltaX * (vectorOld2.sad - vectorOld1.sad);

					int32_t vector2_x = vectorOld3.x * nStepXold + deltaX * (vectorOld4.x - vectorOld3.x);
					int32_t vector2_y = vectorOld3.y * nStepXold + deltaX * (vectorOld4.y - vectorOld3.y);
					auto vector2_sad = vectorOld3.sad * nStepXold + deltaX * (vectorOld4.sad - vectorOld3.sad);

					vectorOld.x = (vector1_x + deltaY * (vector2_x - vector1_x) / nStepYold) / nStepXold;
					vectorOld.y = (vector1_y + deltaY * (vector2_y - vector1_y) / nStepYold) / nStepXold;
					vectorOld.sad = (vector1_sad + deltaY * (vector2_sad - vector1_sad) / nStepYold) / nStepXold;

				}
				else // nearest
				{
					if (deltaX * 2 < nStepXold && deltaY * 2 < nStepYold)
						vectorOld = mvClip[0][blkxold1 + blkyold1 * nBlkXold].GetMV();
					else if (deltaX * 2 >= nStepXold && deltaY * 2 < nStepYold)
						vectorOld = mvClip[0][blkxold2 + blkyold1 * nBlkXold].GetMV();
					else if (deltaX * 2 < nStepXold && deltaY * 2 >= nStepYold)
						vectorOld = mvClip[0][blkxold1 + blkyold2 * nBlkXold].GetMV();
					else //(deltaX*2>=nStepXold && deltaY*2>=nStepYold )
						vectorOld = mvClip[0][blkxold2 + blkyold2 * nBlkXold].GetMV();
				}

				// scale vector to new nPel
				vectorOld.x = vectorOld.x ? vectorOld.x / abs(vectorOld.x) * ((abs(vectorOld.x) << nLogPel) >> nLogPelold) : 0;
				vectorOld.y = vectorOld.y ? vectorOld.y / abs(vectorOld.y) * ((abs(vectorOld.y) << nLogPel) >> nLogPelold) : 0;

				predictor = ClipMV(vectorOld); // predictor
				predictor.sad = vectorOld.sad * (nBlkSizeX * nBlkSizeY) / (nBlkSizeXold * nBlkSizeYold); // normalized to new block size

				bestMV.x = predictor.x;
				bestMV.y = predictor.y;
				bestMV.sad = predictor.sad;

				// update SAD
				if (dctmode != 0) // DCT method (luma only - currently use normal spatial SAD chroma)
				{
					// make dct of source block
					if (dctmode <= 4) //don't do the slow dct conversion if SATD used
						DCT->DCTBytes2D(pSrc[0], nSrcPitch[0], dctSrc, dctpitch);
				}
				if (dctmode >= 3) // most use it and it should be fast anyway //if (dctmode == 3 || dctmode == 4) // check it
					srcLuma = LUMA(pSrc[0], nSrcPitch[0]);

				double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(predictor.x, predictor.y), nRefPitch[1])
					+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(predictor.x, predictor.y), nRefPitch[2]) : 0.f;
				double sad = LumaSAD(GetRefBlock(predictor.x, predictor.y));
				sad += saduv;
				bestMV.sad = sad;
				nMinCost = sad;


				if (bestMV.sad > thSAD)// if old interpolated vector is bad
				{
					// then, we refine, according to the search type
					if (searchType & ONETIME)
						for (int32_t i = nSearchParam; i > 0; i /= 2)
							OneTimeSearch(i);

					if (searchType & NSTEP)
						NStepSearch(nSearchParam);

					if (searchType & LOGARITHMIC)
						for (int32_t i = nSearchParam; i > 0; i /= 2)
							DiamondSearch(i);

					if (searchType & EXHAUSTIVE)
					{
						int32_t mvx = bestMV.x;
						int32_t mvy = bestMV.y;
						for (int32_t i = 1; i <= nSearchParam; i++)// region is same as exhaustive, but ordered by radius (from near to far)
							ExpandingSearch(i, 1, mvx, mvy);
					}

					if (searchType & HEX2SEARCH)
						Hex2Search(nSearchParam);

					if (searchType & UMHSEARCH)
						UMHSearch(nSearchParam, bestMV.x, bestMV.y);

					if (searchType & HSEARCH)
					{
						int32_t mvx = bestMV.x;
						int32_t mvy = bestMV.y;
						for (int32_t i = 1; i <= nSearchParam; i++)
						{
							CheckMV(mvx - i, mvy);
							CheckMV(mvx + i, mvy);
						}
					}

					if (searchType & VSEARCH)
					{
						int32_t mvx = bestMV.x;
						int32_t mvy = bestMV.y;
						for (int32_t i = 1; i <= nSearchParam; i++)
						{
							CheckMV(mvx, mvy - i);
							CheckMV(mvx, mvy + i);
						}
					}
				}

				// we store the result
				vectors[blkIdx].x = bestMV.x;
				vectors[blkIdx].y = bestMV.y;
				vectors[blkIdx].sad = bestMV.sad;


				/* write the results */
				auto& BlockData = reinterpret_cast<VectorStructure&>(pBlkData[blkx * N_PER_BLOCK]);
				BlockData = bestMV;


				if (iblkx < nBlkX - 1)
				{
					x[0] += (nBlkSizeX - nOverlapX) * blkScanDir;
					if (pSrcFrame->GetMode() & UPLANE)
						x[1] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
					if (pSrcFrame->GetMode() & VPLANE)
						x[2] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
				}
			}
			pBlkData += nBlkX * N_PER_BLOCK;

			y[0] += (nBlkSizeY - nOverlapY);
			if (pSrcFrame->GetMode() & UPLANE)
				y[1] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV);
			if (pSrcFrame->GetMode() & VPLANE)
				y[2] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV);
		}
	}
};
