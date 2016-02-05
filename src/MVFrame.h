#ifndef MVTOOLS_MVFRAME_H
#define MVTOOLS_MVFRAME_H
#include <cstdint>
#include <cstdio>

enum MVPlaneSet {
	YPLANE = 1,
	UPLANE = 2,
	VPLANE = 4,
	YUPLANES = 3,
	YVPLANES = 5,
	UVPLANES = 6,
	YUVPLANES = 7
};

class MVPlane {
	uint8_t **pPlane;
	int32_t nWidth;
	int32_t nHeight;
	int32_t nExtendedWidth;
	int32_t nExtendedHeight;
	int32_t nPitch;
	int32_t nHPadding;
	int32_t nVPadding;
	int32_t nOffsetPadding;
	int32_t nHPaddingPel;
	int32_t nVPaddingPel;
	int32_t nPel;
	bool isPadded;
	bool isRefined;
	bool isFilled;
	template <typename PixelType>
	void RefineExtPel2(const uint8_t *pSrc2x, int32_t nSrc2xPitch, bool isExtPadded);
	template <typename PixelType>
	void RefineExtPel4(const uint8_t *pSrc2x, int32_t nSrc2xPitch, bool isExtPadded);
public:
	MVPlane(int32_t _nWidth, int32_t _nHeight, int32_t _nPel, int32_t _nHPad, int32_t _nVPad);
	~MVPlane();
	void Update(uint8_t* pSrc, int32_t _nPitch);
	void ChangePlane(const uint8_t *pNewPlane, int32_t nNewPitch);
	void Pad();
	void Refine(int32_t interType);
	void RefineExt(const uint8_t *pSrc2x, int32_t nSrc2xPitch, bool isExtPadded); //2.0.08
	void ReduceTo(MVPlane *pReducedPlane, int32_t rfilter);
	void WritePlane(FILE *pFile);
	inline const uint8_t *GetAbsolutePointer(int32_t nX, int32_t nY) const {
		if (nPel == 1)
			return pPlane[0] + nX * 4 + nY * nPitch;
		else if (nPel == 2) {
			int32_t idx = (nX & 1) | ((nY & 1) << 1);
			nX >>= 1;
			nY >>= 1;
			return pPlane[idx] + nX * 4 + nY * nPitch;
		}
		else {
			int32_t idx = (nX & 3) | ((nY & 3) << 2);
			nX >>= 2;
			nY >>= 2;
			return pPlane[idx] + nX * 4 + nY * nPitch;
		}
	}
	inline const uint8_t *GetAbsolutePointerPel1(int32_t nX, int32_t nY) const {
		return pPlane[0] + nX * 4 + nY * nPitch;
	}
	inline const uint8_t *GetAbsolutePointerPel2(int32_t nX, int32_t nY) const {
		int32_t idx = (nX & 1) | ((nY & 1) << 1);
		nX >>= 1;
		nY >>= 1;
		return pPlane[idx] + nX * 4 + nY * nPitch;
	}
	inline const uint8_t *GetAbsolutePointerPel4(int32_t nX, int32_t nY) const {
		int32_t idx = (nX & 3) | ((nY & 3) << 2);
		nX >>= 2;
		nY >>= 2;
		return pPlane[idx] + nX * 4 + nY * nPitch;
	}
	inline const uint8_t *GetPointer(int32_t nX, int32_t nY) const {
		return GetAbsolutePointer(nX + nHPaddingPel, nY + nVPaddingPel);
	}
	inline const uint8_t *GetPointerPel1(int32_t nX, int32_t nY) const {
		return GetAbsolutePointerPel1(nX + nHPaddingPel, nY + nVPaddingPel);
	}
	inline const uint8_t *GetPointerPel2(int32_t nX, int32_t nY) const {
		return GetAbsolutePointerPel2(nX + nHPaddingPel, nY + nVPaddingPel);
	}
	inline const uint8_t *GetPointerPel4(int32_t nX, int32_t nY) const {
		return GetAbsolutePointerPel4(nX + nHPaddingPel, nY + nVPaddingPel);
	}
	inline const uint8_t *GetAbsolutePelPointer(int32_t nX, int32_t nY) const {
		return pPlane[0] + nX * 4 + nY * nPitch;
	}
	inline int32_t GetPitch() const { return nPitch; }
	inline int32_t GetWidth() const { return nWidth; }
	inline int32_t GetHeight() const { return nHeight; }
	inline int32_t GetExtendedWidth() const { return nExtendedWidth; }
	inline int32_t GetExtendedHeight() const { return nExtendedHeight; }
	inline int32_t GetHPadding() const { return nHPadding; }
	inline int32_t GetVPadding() const { return nVPadding; }
	inline void ResetState() { isRefined = isFilled = isPadded = false; }
};

class MVFrame {
	MVPlane *pYPlane;
	MVPlane *pUPlane;
	MVPlane *pVPlane;
	int32_t nMode;
	int32_t xRatioUV;
	int32_t yRatioUV;
public:
	MVFrame(int32_t nWidth, int32_t nHeight, int32_t nPel, int32_t nHPad, int32_t nVPad, int32_t _nMode, int32_t _xRatioUV, int32_t _yRatioUV);
	~MVFrame();
	void Update(int32_t _nMode, uint8_t * pSrcY, int32_t pitchY, uint8_t * pSrcU, int32_t pitchU, uint8_t *pSrcV, int32_t pitchV);
	void ChangePlane(const uint8_t *pNewSrc, int32_t nNewPitch, MVPlaneSet _nMode);
	void Refine(MVPlaneSet _nMode, int32_t interType);
	void Pad(MVPlaneSet _nMode);
	void ReduceTo(MVFrame *pFrame, MVPlaneSet _nMode, int32_t rfilter);
	void ResetState();
	void WriteFrame(FILE *pFile);
	inline MVPlane *GetPlane(MVPlaneSet _nMode) {
		if (_nMode & YPLANE)
			return pYPlane;
		if (_nMode & UPLANE)
			return pUPlane;
		if (_nMode & VPLANE)
			return pVPlane;
		return 0;
	}
	inline int32_t GetMode() { return nMode; }
};

class MVGroupOfFrames {
	int32_t nLevelCount;
	MVFrame **pFrames;
	int32_t nWidth;
	int32_t nHeight;
	int32_t nPel;
	int32_t nHPad;
	int32_t nVPad;
	int32_t xRatioUV;
	int32_t yRatioUV;
public:
	MVGroupOfFrames(int32_t _nLevelCount, int32_t nWidth, int32_t nHeight, int32_t nPel, int32_t nHPad, int32_t nVPad, int32_t nMode, int32_t _xRatioUV, int32_t yRatioUV);
	~MVGroupOfFrames();
	void Update(int32_t nModeYUV, uint8_t * pSrcY, int32_t pitchY, uint8_t * pSrcU, int32_t pitchU, uint8_t *pSrcV, int32_t pitchV);
	MVFrame *GetFrame(int32_t nLevel);
	void SetPlane(const uint8_t *pNewSrc, int32_t nNewPitch, MVPlaneSet nMode);
	void Refine(MVPlaneSet nMode, int32_t interType);
	void Pad(MVPlaneSet nMode);
	void Reduce(MVPlaneSet nMode, int32_t rfilter);
	void ResetState();
};

#endif
