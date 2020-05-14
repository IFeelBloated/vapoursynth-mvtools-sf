#ifndef __MV_INTERFACES_H__
#define __MV_INTERFACES_H__

#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#define MIN(a,b) (((a) > (b)) ? (b) : (a))
#define MOTION_IS_BACKWARD          0x00000002
#define MOTION_SMALLEST_PLANE       0x00000004
#define MOTION_USE_CHROMA_MOTION    0x00000008
#define MOTION_USE_SSD              0x00000010
#define MOTION_USE_SATD             0x00000020

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "Include/Interface.hxx"

constexpr auto MV_DEFAULT_SCD1 = 400.;
constexpr auto MV_DEFAULT_SCD2 = 130.;
constexpr auto MotionMagicKey = 0x564D;
constexpr auto MVAnalysisDataVersion = 5;

struct VectorStructure {
	self(x, 0_i32);
	self(y, 0_i32);
	self(sad, -1.f);
};

constexpr auto N_PER_BLOCK = sizeof(VectorStructure) / sizeof(std::int32_t);

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

auto zeroMV = VectorStructure{};

class MVAnalysisData {
public:
	int32_t nMagicKey;
	int32_t nVersion;
	int32_t nBlkSizeX;
	int32_t nBlkSizeY;
	int32_t nPel;
	int32_t nLvCount;
	int32_t nDeltaFrame;
	bool isBackward;
	int32_t nMotionFlags;
	int32_t nWidth;
	int32_t nHeight;
	int32_t nOverlapX;
	int32_t nOverlapY;
	int32_t nBlkX;
	int32_t nBlkY;
	int32_t yRatioUV;
	int32_t xRatioUV;
	int32_t nHPadding;
	int32_t nVPadding;
public:
	inline void SetMotionFlags(int32_t _nMotionFlags) { nMotionFlags |= _nMotionFlags; }
	inline int32_t GetMotionFlags() const { return nMotionFlags; }
	inline int32_t GetBlkSizeX() const { return nBlkSizeX; }
	inline int32_t GetPel() const { return nPel; }
	inline int32_t GetLevelCount() const { return nLvCount; }
	inline bool IsBackward() const { return isBackward; }
	inline int32_t GetMagicKey() const { return nMagicKey; }
	inline int32_t GetDeltaFrame() const { return nDeltaFrame; }
	inline int32_t GetWidth() const { return nWidth; }
	inline int32_t GetHeight() const { return nHeight; }
	inline bool IsChromaMotion() const { return !!(nMotionFlags & MOTION_USE_CHROMA_MOTION); }
	inline int32_t GetOverlapX() const { return nOverlapX; }
	inline int32_t GetBlkX() const { return nBlkX; }
	inline int32_t GetBlkY() const { return nBlkY; }
	inline int32_t GetYRatioUV() const { return yRatioUV; }
	inline int32_t GetXRatioUV() const { return xRatioUV; }
	inline int32_t GetBlkSizeY() const { return nBlkSizeY; }
	inline int32_t GetOverlapY() const { return nOverlapY; }
	inline int32_t GetHPadding() const { return nHPadding; }
	inline int32_t GetVPadding() const { return nVPadding; }
};

class MVException : public std::runtime_error {
public:
	MVException(const char *descr) : std::runtime_error(descr) {}
	MVException(const std::string &descr) : std::runtime_error(descr) {}
};

#endif
