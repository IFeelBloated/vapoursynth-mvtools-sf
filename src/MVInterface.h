#ifndef __MV_INTERFACES_H__
#define __MV_INTERFACES_H__
#define MOTION_MAGIC_KEY 0x564D //'MV' is IMHO better 31415926 :)
#define N_PER_BLOCK 3
#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#define MIN(a,b) (((a) > (b)) ? (b) : (a))
#define MOTION_IS_BACKWARD          0x00000002
#define MOTION_SMALLEST_PLANE       0x00000004
#define MOTION_USE_CHROMA_MOTION    0x00000008
#define MOTION_USE_SSD              0x00000010
#define MOTION_USE_SATD             0x00000020
#define MV_DEFAULT_SCD1             400.f
#define MV_DEFAULT_SCD2             130.f
#define MVANALYSIS_DATA_VERSION 5
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "VapourSynth.h"

struct VECTOR {
	int x;
	int y;
	int sad;
};

inline void CopyVector(VECTOR *destVector, const VECTOR *srcVector) {
	destVector->x = srcVector->x;
	destVector->y = srcVector->y;
	destVector->sad = srcVector->sad;
}

enum SearchType {
	ONETIME = 1,
	NSTEP = 2,
	LOGARITHMIC = 4,
	EXHAUSTIVE = 8,
	HEX2SEARCH = 16,
	UMHSEARCH = 32,
	HSEARCH = 64,
	VSEARCH = 128
};

static const VECTOR zeroMV = { 0, 0, -1 };

class MVAnalysisData {
public:
	int nMagicKey;
	int nVersion;
	int nBlkSizeX;
	int nBlkSizeY;
	int nPel;
	int nLvCount;
	int nDeltaFrame;
	bool isBackward;
	int nMotionFlags;
	int nWidth;
	int nHeight;
	int nOverlapX;
	int nOverlapY;
	int nBlkX;
	int nBlkY;
	int yRatioUV;
	int xRatioUV;
	int nHPadding;
	int nVPadding;
public:
	inline void SetMotionFlags(int _nMotionFlags) { nMotionFlags |= _nMotionFlags; }
	inline int GetMotionFlags() const { return nMotionFlags; }
	inline int GetBlkSizeX() const { return nBlkSizeX; }
	inline int GetPel() const { return nPel; }
	inline int GetLevelCount() const { return nLvCount; }
	inline bool IsBackward() const { return isBackward; }
	inline int GetMagicKey() const { return nMagicKey; }
	inline int GetDeltaFrame() const { return nDeltaFrame; }
	inline int GetWidth() const { return nWidth; }
	inline int GetHeight() const { return nHeight; }
	inline bool IsChromaMotion() const { return !!(nMotionFlags & MOTION_USE_CHROMA_MOTION); }
	inline int GetOverlapX() const { return nOverlapX; }
	inline int GetBlkX() const { return nBlkX; }
	inline int GetBlkY() const { return nBlkY; }
	inline int GetYRatioUV() const { return yRatioUV; }
	inline int GetXRatioUV() const { return xRatioUV; }
	inline int GetBlkSizeY() const { return nBlkSizeY; }
	inline int GetOverlapY() const { return nOverlapY; }
	inline int GetHPadding() const { return nHPadding; }
	inline int GetVPadding() const { return nVPadding; }
};

class MVException : public std::runtime_error {
public:
	MVException(const char *descr) : std::runtime_error(descr) {}
	MVException(const std::string &descr) : std::runtime_error(descr) {}
};

#endif
