#ifndef __POBLOCKS__
#define __POBLOCKS__
#define MAX_PREDICTOR 20
#define ALIGN_PLANES 64
#define ALIGN_SOURCEBLOCK 16
#define ALLOW_DCT
#include <cstdlib>
#include "MVClip.h"
#include "MVFrame.h"
#include "Interpolation.h"
#include "CopyCode.h"
#include "SADFunctions.h"
#include "CommonFunctions.h"
#include "Variance.h"
#include "FakeSAD.h"
#include "DCT.h"

class PlaneOfBlocks {
	int nBlkX;
	int nBlkY;
	int nBlkSizeX;
	int nBlkSizeY;
	int nBlkCount;
	int nPel;
	int nLogPel;
	int nScale;
	int nLogScale;
	int nMotionFlags;
	int nOverlapX;
	int nOverlapY;
	int xRatioUV;
	int yRatioUV;
	int nLogxRatioUV;
	int nLogyRatioUV;
	SADFunction SAD;
	LUMAFunction LUMA;
	COPYFunction BLITLUMA;
	COPYFunction BLITCHROMA;
	SADFunction SADCHROMA;
	SADFunction SATD;
	VECTOR *vectors;
	bool smallestPlane;
	bool chroma;
	MVFrame *pSrcFrame;
	MVFrame *pRefFrame;
	int nSrcPitch[3];
	const uint8_t* pSrc[3];
	int nRefPitch[3];
	VECTOR bestMV;
	int nBestSad;
	float nMinCost;
	VECTOR predictor;
	VECTOR predictors[MAX_PREDICTOR];
	int nDxMin;
	int nDyMin;
	int nDxMax;
	int nDyMax;
	int x[3];
	int y[3];
	int blkx;
	int blky;
	int blkIdx;
	int blkScanDir;
	SearchType searchType;
	int nSearchParam;
	double nLambda;
	double LSAD;
	int penaltyNew;
	int penaltyZero;
	int pglobal;
	double badSAD;
	int badrange;
	double planeSAD;
	int badcount;
	bool temporal;
	bool tryMany;
	int iter;
	VECTOR globalMVPredictor;
	VECTOR zeroMVfieldShifted;
	DCTClass * DCT;
	uint8_t * dctSrc;
	uint8_t * dctRef;
	int dctpitch;
	int dctmode;
	float srcLuma;
	float refLuma;
	float sumLumaChange;
	float dctweight16;
	int *freqArray;
	int freqSize;
	float verybigSAD;
	int nSrcPitch_temp[3];
	uint8_t* pSrc_temp[3];
	inline const uint8_t *GetRefBlock(int nVx, int nVy) {
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
	inline const uint8_t *GetRefBlockU(int nVx, int nVy) {
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
	inline const uint8_t *GetRefBlockV(int nVx, int nVy) {
		if (nPel == 2)
			return pRefFrame->GetPlane(VPLANE)->GetAbsolutePointerPel2(
				x[1] * 2 + nVx / xRatioUV,
				y[1] * 2 + nVy / yRatioUV);
		else if (nPel == 1)
			return pRefFrame->GetPlane(VPLANE)->GetAbsolutePointerPel1(
				x[1] + nVx / xRatioUV,
				y[1] + nVy / yRatioUV);
		else
			return pRefFrame->GetPlane(VPLANE)->GetAbsolutePointerPel4(
				x[1] * 4 + nVx / xRatioUV,
				y[1] * 4 + nVy / yRatioUV);
	}
	inline const uint8_t *GetSrcBlock(int nX, int nY) {
		return pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(nX, nY);
	}
	inline float MotionDistorsion(int vx, int vy) {
		int dist = SquareDifferenceNorm(predictor, vx, vy);
		return static_cast<float>((nLambda * dist) / 256.f);
	}
	float LumaSADx(const uint8_t *pRef0) {
		float sad;
		switch (dctmode) {
		case 1:
			DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
			{
				float *dctSrc16 = (float *)dctSrc;
				float *dctRef16 = (float *)dctRef;
				sad = (SAD(dctSrc, dctpitch, dctRef, dctpitch) + abs(dctSrc16[0] - dctRef16[0]) * 3) * nBlkSizeX / 2;
			}
			break;
		case 2:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 0) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				float dctsad;
				{
					float *dctSrc16 = (float *)dctSrc;
					float *dctRef16 = (float *)dctRef;
					dctsad = (SAD(dctSrc, dctpitch, dctRef, dctpitch) + abs(dctSrc16[0] - dctRef16[0]) * 3)*nBlkSizeX / 2;
				}
				sad = (sad*(16 - dctweight16) + dctsad*dctweight16) / 16;
			}
			break;
		case 3:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				float dctsad = SAD(dctSrc, dctpitch, dctRef, dctpitch)*nBlkSizeX / 2;
				sad = sad / 2 + dctsad / 2;
			}
			break;
		case 4:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				float dctsad = SAD(dctSrc, dctpitch, dctRef, dctpitch)*nBlkSizeX / 2;
				sad = sad / 4 + dctsad / 2 + dctsad / 4;
			}
			break;
		case 5:
			sad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			break;
		case 6:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 0) {
				float dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = (sad*(16 - dctweight16) + dctsad*dctweight16) / 16;
			}
			break;
		case 7:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				float dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 2 + dctsad / 2;
			}
			break;
		case 8:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				float dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 4 + dctsad / 2 + dctsad / 4;
			}
			break;
		case 9:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 1) {
				float dctweighthalf = dctweight16 / 2;
				float dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = (sad*(16 - dctweighthalf) + dctsad*dctweighthalf) / 16;
			}
			break;
		case 10:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 16) {
				float dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 2 + dctsad / 4 + sad / 4;
			}
			break;
		default:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
		}
		return sad;
	}
	inline float LumaSAD(const uint8_t *pRef0) {
		return !dctmode ? SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]) : LumaSADx(pRef0);
	}
	inline void CheckMV0(int vx, int vy) {
		if (IsVectorOK(vx, vy)) {
			float cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			float sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad;
			if (cost >= nMinCost) return;
			float saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv;
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = FakeInt(sad + saduv);
		}
	}
	inline void CheckMV(int vx, int vy) {
		if (IsVectorOK(vx, vy)) {
			float cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			float sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew*sad) / 256);
			if (cost >= nMinCost) return;
			float saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew*saduv) / 256);
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = FakeInt(sad + saduv);
		}
	}
	inline void CheckMV2(int vx, int vy, int *dir, int val) {
		if (IsVectorOK(vx, vy)) {
			float cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			float sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew*sad) / 256);
			if (cost >= nMinCost) return;
			float saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew*saduv) / 256);
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = FakeInt(sad + saduv);
			*dir = val;
		}
	}
	inline void CheckMVdir(int vx, int vy, int *dir, int val) {
		if (IsVectorOK(vx, vy)) {
			float cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			float sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew*sad) / 256);
			if (cost >= nMinCost) return;
			float saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew*saduv) / 256);
			if (cost >= nMinCost) return;
			nMinCost = cost;
			bestMV.sad = FakeInt(sad + saduv);
			*dir = val;
		}
	}
	inline int ClipMVx(int vx) {
		if (vx < nDxMin) return nDxMin;
		else if (vx >= nDxMax) return nDxMax - 1;
		else return vx;
	}
	inline int ClipMVy(int vy) {
		if (vy < nDyMin) return nDyMin;
		else if (vy >= nDyMax) return nDyMax - 1;
		else return vy;
	}
	inline VECTOR ClipMV(VECTOR v) {
		VECTOR v2;
		v2.x = ClipMVx(v.x);
		v2.y = ClipMVy(v.y);
		v2.sad = v.sad;
		return v2;
	}
	static inline int Median(int a, int b, int c) {
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
	inline  bool IsVectorOK(int vx, int vy) {
		return ((vx >= nDxMin) &&
			(vy >= nDyMin) &&
			(vx < nDxMax) &&
			(vy < nDyMax));
	}
	static inline unsigned int SquareDifferenceNorm(const VECTOR& v1, const VECTOR& v2) {
		return (v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y);
	}
	static inline unsigned int SquareDifferenceNorm(const VECTOR& v1, const int v2x, const int v2y) {
		return (v1.x - v2x) * (v1.x - v2x) + (v1.y - v2y) * (v1.y - v2y);
	}
	inline bool IsInFrame(int i) {
		return ((i >= 0) && (i < nBlkCount));
	}
	void Refine();
public:
	PlaneOfBlocks(int _nBlkX, int _nBlkY, int _nBlkSizeX, int _nBlkSizeY, int _nPel, int _nLevel, int _nMotionFlags, int _nOverlapX, int _nOverlapY, int _xRatioUV, int _yRatioUV);
	~PlaneOfBlocks();
	void FetchPredictors();
	void DiamondSearch(int step);
	void NStepSearch(int stp);
	void OneTimeSearch(int length);
	void PseudoEPZSearch();
	void ExpandingSearch(int radius, int step, int mvx, int mvy);
	void Hex2Search(int i_me_range);
	void CrossSearch(int start, int x_max, int y_max, int mvx, int mvy);
	void UMHSearch(int i_me_range, int omx, int omy);
	void SearchMVs(MVFrame *_pSrcFrame, MVFrame *_pRefFrame, SearchType st,
		int stp, double _lambda, double _lSAD, int _pennew, int _plevel, int *out, VECTOR *globalMVec, short * outfilebuf, int _fieldShiftCur,
		DCTClass * _DCT, float * _meanLumaChange, int _divideExtra,
		int _pzero, int _pglobal, double badSAD, int badrange, bool meander, int *vecPrev, bool tryMany);
	void InterpolatePrediction(const PlaneOfBlocks &pob);
	void WriteHeaderToArray(int *array);
	int WriteDefaultToArray(int *array, int divideExtra);
	int GetArraySize(int divideExtra);
	void EstimateGlobalMVDoubled(VECTOR *globalMVDoubled);
	inline int GetnBlkX() { return nBlkX; }
	inline int GetnBlkY() { return nBlkY; }
	void RecalculateMVs(MVClipBalls & mvClip, MVFrame *_pSrcFrame, MVFrame *_pRefFrame, SearchType st,
		int stp, float _lambda, int _pennew,
		int *out, short * outfilebuf, int _fieldShiftCur, float thSAD, DCTClass * _DCT,
		int _divideExtra, int smooth, bool meander);
};

#endif
