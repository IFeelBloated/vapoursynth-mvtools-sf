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
#include "DCT.h"

class PlaneOfBlocks {
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
	VECTOR *vectors;
	bool smallestPlane;
	bool chroma;
	MVFrame *pSrcFrame;
	MVFrame *pRefFrame;
	int32_t nSrcPitch[3];
	const uint8_t* pSrc[3];
	int32_t nRefPitch[3];
	VECTOR bestMV;
	int32_t nBestSad;
	double nMinCost;
	VECTOR predictor;
	VECTOR predictors[MAX_PREDICTOR];
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
	VECTOR globalMVPredictor;
	VECTOR zeroMVfieldShifted;
	DCTClass * DCT;
	uint8_t * dctSrc;
	uint8_t * dctRef;
	int32_t dctpitch;
	int32_t dctmode;
	double srcLuma;
	double refLuma;
	double sumLumaChange;
	double dctweight16;
	int32_t *freqArray;
	int32_t freqSize;
	double verybigSAD;
	int32_t nSrcPitch_temp[3];
	uint8_t* pSrc_temp[3];
	inline const uint8_t *GetRefBlock(int32_t nVx, int32_t nVy) {
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
	inline const uint8_t *GetRefBlockU(int32_t nVx, int32_t nVy) {
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
	inline const uint8_t *GetRefBlockV(int32_t nVx, int32_t nVy) {
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
	inline const uint8_t *GetSrcBlock(int32_t nX, int32_t nY) {
		return pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(nX, nY);
	}
	inline double MotionDistorsion(int32_t vx, int32_t vy) {
		int32_t dist = SquareDifferenceNorm(predictor, vx, vy);
		return (nLambda * dist) / 256.;
	}
	double LumaSADx(const uint8_t *pRef0) {
		double sad;
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
				double dctsad;
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
				double dctsad = SAD(dctSrc, dctpitch, dctRef, dctpitch)*nBlkSizeX / 2;
				sad = sad / 2 + dctsad / 2;
			}
			break;
		case 4:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				DCT->DCTBytes2D(pRef0, nRefPitch[0], dctRef, dctpitch);
				double dctsad = SAD(dctSrc, dctpitch, dctRef, dctpitch)*nBlkSizeX / 2;
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
				sad = (sad*(16 - dctweight16) + dctsad*dctweight16) / 16;
			}
			break;
		case 7:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 2 + dctsad / 2;
			}
			break;
		case 8:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 32) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 4 + dctsad / 2 + dctsad / 4;
			}
			break;
		case 9:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (dctweight16 > 1) {
				double dctweighthalf = dctweight16 / 2;
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = (sad*(16 - dctweighthalf) + dctsad*dctweighthalf) / 16;
			}
			break;
		case 10:
			refLuma = LUMA(pRef0, nRefPitch[0]);
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
			if (abs(srcLuma - refLuma) > (srcLuma + refLuma) / 16) {
				double dctsad = SATD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
				sad = sad / 2 + dctsad / 4 + sad / 4;
			}
			break;
		default:
			sad = SAD(pSrc[0], nSrcPitch[0], pRef0, nRefPitch[0]);
		}
		return sad;
	}
	inline double LumaSAD(const uint8_t *pRef0) {
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
			bestMV.sad = _fakeint(static_cast<float>(sad + saduv));
		}
	}
	inline void CheckMV(int32_t vx, int32_t vy) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew*sad) / 256);
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew*saduv) / 256);
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = _fakeint(static_cast<float>(sad + saduv));
		}
	}
	inline void CheckMV2(int32_t vx, int32_t vy, int32_t *dir, int32_t val) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew*sad) / 256);
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew*saduv) / 256);
			if (cost >= nMinCost) return;
			bestMV.x = vx;
			bestMV.y = vy;
			nMinCost = cost;
			bestMV.sad = _fakeint(static_cast<float>(sad + saduv));
			*dir = val;
		}
	}
	inline void CheckMVdir(int32_t vx, int32_t vy, int32_t *dir, int32_t val) {
		if (IsVectorOK(vx, vy)) {
			double cost = MotionDistorsion(vx, vy);
			if (cost >= nMinCost) return;
			double sad = LumaSAD(GetRefBlock(vx, vy));
			cost += sad + ((penaltyNew*sad) / 256);
			if (cost >= nMinCost) return;
			double saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(vx, vy), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(vx, vy), nRefPitch[2]) : 0;
			cost += saduv + ((penaltyNew*saduv) / 256);
			if (cost >= nMinCost) return;
			nMinCost = cost;
			bestMV.sad = _fakeint(static_cast<float>(sad + saduv));
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
	inline VECTOR ClipMV(VECTOR v) {
		VECTOR v2;
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
	inline  bool IsVectorOK(int32_t vx, int32_t vy) {
		return ((vx >= nDxMin) &&
			(vy >= nDyMin) &&
			(vx < nDxMax) &&
			(vy < nDyMax));
	}
	static inline uint32_t SquareDifferenceNorm(const VECTOR& v1, const VECTOR& v2) {
		return (v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y);
	}
	static inline uint32_t SquareDifferenceNorm(const VECTOR& v1, const int32_t v2x, const int32_t v2y) {
		return (v1.x - v2x) * (v1.x - v2x) + (v1.y - v2y) * (v1.y - v2y);
	}
	inline bool IsInFrame(int32_t i) {
		return ((i >= 0) && (i < nBlkCount));
	}
	void Refine();
public:
	PlaneOfBlocks(int32_t _nBlkX, int32_t _nBlkY, int32_t _nBlkSizeX, int32_t _nBlkSizeY, int32_t _nPel, int32_t _nLevel, int32_t _nMotionFlags, int32_t _nOverlapX, int32_t _nOverlapY, int32_t _xRatioUV, int32_t _yRatioUV);
	~PlaneOfBlocks();
	void FetchPredictors();
	void DiamondSearch(int32_t step);
	void NStepSearch(int32_t stp);
	void OneTimeSearch(int32_t length);
	void PseudoEPZSearch();
	void ExpandingSearch(int32_t radius, int32_t step, int32_t mvx, int32_t mvy);
	void Hex2Search(int32_t i_me_range);
	void CrossSearch(int32_t start, int32_t x_max, int32_t y_max, int32_t mvx, int32_t mvy);
	void UMHSearch(int32_t i_me_range, int32_t omx, int32_t omy);
	void SearchMVs(MVFrame *_pSrcFrame, MVFrame *_pRefFrame, SearchType st,
		int32_t stp, double _lambda, double _lSAD, int32_t _pennew, int32_t _plevel, int32_t *out, VECTOR *globalMVec, int32_t * outfilebuf, int32_t _fieldShiftCur,
		DCTClass * _DCT, double *_meanLumaChange, int32_t _divideExtra,
		int32_t _pzero, int32_t _pglobal, double badSAD, int32_t badrange, bool meander, int32_t *vecPrev, bool tryMany);
	void InterpolatePrediction(const PlaneOfBlocks &pob);
	void WriteHeaderToArray(int32_t *array);
	int32_t WriteDefaultToArray(int32_t *array, int32_t divideExtra);
	int32_t GetArraySize(int32_t divideExtra);
	void EstimateGlobalMVDoubled(VECTOR *globalMVDoubled);
	inline int32_t GetnBlkX() { return nBlkX; }
	inline int32_t GetnBlkY() { return nBlkY; }
	void RecalculateMVs(MVClipBalls & mvClip, MVFrame *_pSrcFrame, MVFrame *_pRefFrame, SearchType st,
		int32_t stp, double _lambda, int32_t _pennew,
		int32_t *out, int32_t *outfilebuf, int32_t _fieldShiftCur, double thSAD, DCTClass *_DCT,
		int32_t _divideExtra, int32_t smooth, bool meander);
};

#endif
