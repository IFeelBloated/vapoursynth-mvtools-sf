#pragma once
#include <cstdint>
#include <cstdio>
#include "VSHelper.h"
#include "Padding.h"
#include "Interpolation.h"

auto PlaneHeightLuma(int32_t src_height, int32_t level, int32_t yRatioUV, int32_t vpad) {
	int32_t height = src_height;
	for (int32_t i = 1; i <= level; i++)
		height = vpad >= yRatioUV ? ((height / yRatioUV + 1) / 2) * yRatioUV : ((height / yRatioUV) / 2) * yRatioUV;
	return height;
}

auto PlaneWidthLuma(int32_t src_width, int32_t level, int32_t xRatioUV, int32_t hpad) {
	int32_t width = src_width;
	for (int32_t i = 1; i <= level; i++)
		width = hpad >= xRatioUV ? ((width / xRatioUV + 1) / 2) * xRatioUV : ((width / xRatioUV) / 2) * xRatioUV;
	return width;
}

auto PlaneSuperOffset(bool chroma, int32_t src_height, int32_t level, int32_t pel, int32_t vpad, int32_t plane_pitch, int32_t yRatioUV) {
	int32_t height = src_height;
	uint32_t offset;
	if (level == 0)
		offset = 0;
	else {
		offset = pel * pel * plane_pitch * (src_height + vpad * 2);
		for (int32_t i = 1; i < level; i++) {
			height = chroma ? PlaneHeightLuma(src_height * yRatioUV, i, yRatioUV, vpad * yRatioUV) / yRatioUV : PlaneHeightLuma(src_height, i, yRatioUV, vpad);
			offset += plane_pitch * (height + vpad * 2);
		}
	}
	return offset;
}

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
	void RefineExtPel2(const uint8_t* pSrc2x8, int32_t nSrc2xPitch, bool isExtPadded) {
		const PixelType* pSrc2x = (const PixelType*)pSrc2x8;
		PixelType* pp1 = (PixelType*)pPlane[1];
		PixelType* pp2 = (PixelType*)pPlane[2];
		PixelType* pp3 = (PixelType*)pPlane[3];

		nSrc2xPitch /= sizeof(PixelType);
		int32_t nPitchTmp = nPitch / sizeof(PixelType);

		// pel clip may be already padded (i.e. is finest clip)
		if (!isExtPadded) {
			int32_t offset = nPitchTmp * nVPadding + nHPadding;
			pp1 += offset;
			pp2 += offset;
			pp3 += offset;
		}

		for (int32_t h = 0; h < nHeight; h++) {// assembler optimization?
			for (int32_t w = 0; w < nWidth; w++) {
				pp1[w] = pSrc2x[(w << 1) + 1];
				pp2[w] = pSrc2x[(w << 1) + nSrc2xPitch];
				pp3[w] = pSrc2x[(w << 1) + nSrc2xPitch + 1];
			}
			pp1 += nPitchTmp;
			pp2 += nPitchTmp;
			pp3 += nPitchTmp;
			pSrc2x += nSrc2xPitch * 2;
		}

		if (!isExtPadded) {
			PadReferenceFrame<PixelType>(pPlane[1], nPitch, nHPadding, nVPadding, nWidth, nHeight);
			PadReferenceFrame<PixelType>(pPlane[2], nPitch, nHPadding, nVPadding, nWidth, nHeight);
			PadReferenceFrame<PixelType>(pPlane[3], nPitch, nHPadding, nVPadding, nWidth, nHeight);
		}
		isPadded = true;
	}
	template <typename PixelType>
	void RefineExtPel4(const uint8_t* pSrc2x8, int32_t nSrc2xPitch, bool isExtPadded) {
		const PixelType* pSrc2x = (const PixelType*)pSrc2x8;
		PixelType* pp[16];
		for (int32_t i = 1; i < 16; i++)
			pp[i] = (PixelType*)pPlane[i];

		nSrc2xPitch /= sizeof(PixelType);
		int32_t nPitchTmp = nPitch / sizeof(PixelType);

		if (!isExtPadded) {
			int32_t offset = nPitchTmp * nVPadding + nHPadding;
			for (int32_t i = 1; i < 16; i++)
				pp[i] += offset;
		}

		for (int32_t h = 0; h < nHeight; h++) {// assembler optimization?
			for (int32_t w = 0; w < nWidth; w++) {
				pp[1][w] = pSrc2x[(w << 2) + 1];
				pp[2][w] = pSrc2x[(w << 2) + 2];
				pp[3][w] = pSrc2x[(w << 2) + 3];
				pp[4][w] = pSrc2x[(w << 2) + nSrc2xPitch];
				pp[5][w] = pSrc2x[(w << 2) + nSrc2xPitch + 1];
				pp[6][w] = pSrc2x[(w << 2) + nSrc2xPitch + 2];
				pp[7][w] = pSrc2x[(w << 2) + nSrc2xPitch + 3];
				pp[8][w] = pSrc2x[(w << 2) + nSrc2xPitch * 2];
				pp[9][w] = pSrc2x[(w << 2) + nSrc2xPitch * 2 + 1];
				pp[10][w] = pSrc2x[(w << 2) + nSrc2xPitch * 2 + 2];
				pp[11][w] = pSrc2x[(w << 2) + nSrc2xPitch * 2 + 3];
				pp[12][w] = pSrc2x[(w << 2) + nSrc2xPitch * 3];
				pp[13][w] = pSrc2x[(w << 2) + nSrc2xPitch * 3 + 1];
				pp[14][w] = pSrc2x[(w << 2) + nSrc2xPitch * 3 + 2];
				pp[15][w] = pSrc2x[(w << 2) + nSrc2xPitch * 3 + 3];
			}
			for (int32_t i = 1; i < 16; i++)
				pp[i] += nPitchTmp;
			pSrc2x += nSrc2xPitch * 4;
		}
		if (!isExtPadded) {
			for (int32_t i = 1; i < 16; i++)
				PadReferenceFrame<PixelType>(pPlane[i], nPitch, nHPadding, nVPadding, nWidth, nHeight);
		}
		isPadded = true;
	}
public:
	MVPlane(int32_t _nWidth, int32_t _nHeight, int32_t _nPel, int32_t _nHPad, int32_t _nVPad) {
		nWidth = _nWidth;
		nHeight = _nHeight;
		nPel = _nPel;
		nHPadding = _nHPad;
		nVPadding = _nVPad;
		nHPaddingPel = _nHPad * nPel;
		nVPaddingPel = _nVPad * nPel;

		nExtendedWidth = nWidth + 2 * nHPadding;
		nExtendedHeight = nHeight + 2 * nVPadding;
		pPlane = new uint8_t * [nPel * nPel];
	}
	~MVPlane() {
		delete[] pPlane;
	}
	void Update(uint8_t* pSrc, int32_t _nPitch) {
		nPitch = _nPitch;
		nOffsetPadding = nPitch * nVPadding + nHPadding * 4;

		for (int32_t i = 0; i < nPel * nPel; i++)
			pPlane[i] = pSrc + i * nPitch * nExtendedHeight;

		ResetState();
		//   LeaveCriticalSection(&cs);
	}
	void ChangePlane(const uint8_t* pNewPlane, int32_t nNewPitch) {
		//    EnterCriticalSection(&cs);
		if (!isFilled)
			vs_bitblt(pPlane[0] + nOffsetPadding, nPitch, pNewPlane, nNewPitch, nWidth * 4, nHeight);
		isFilled = true;
		//   LeaveCriticalSection(&cs);
	}
	void Pad() {
		//    EnterCriticalSection(&cs);
		if (!isPadded) {
			PadReferenceFrame<float>(pPlane[0], nPitch, nHPadding, nVPadding, nWidth, nHeight);
		}

		isPadded = true;
		//   LeaveCriticalSection(&cs);
	}
	void Refine(int32_t sharp) {
		//    EnterCriticalSection(&cs);
		if ((nPel == 2) && (!isRefined))
		{
			if (sharp == 0) // bilinear
			{
				HorizontalBilinear<float>(pPlane[1], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				VerticalBilinear<float>(pPlane[2], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				DiagonalBilinear<float>(pPlane[3], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
			}
			else if (sharp == 1) // bicubic
			{
				{
					HorizontalBicubic<float>(pPlane[1], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
					VerticalBicubic<float>(pPlane[2], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
					HorizontalBicubic<float>(pPlane[3], pPlane[2], nPitch, nPitch, nExtendedWidth, nExtendedHeight); // faster from ready-made horizontal

				}
			}
			else // Wiener
			{

				HorizontalWiener<float>(pPlane[1], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				VerticalWiener<float>(pPlane[2], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				HorizontalWiener<float>(pPlane[3], pPlane[2], nPitch, nPitch, nExtendedWidth, nExtendedHeight); // faster from ready-made horizontal

			}
		}
		else if ((nPel == 4) && (!isRefined)) // firstly pel2 interpolation
		{
			if (sharp == 0) // bilinear
			{

				HorizontalBilinear<float>(pPlane[2], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				VerticalBilinear<float>(pPlane[8], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				DiagonalBilinear<float>(pPlane[10], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);

			}
			else if (sharp == 1) // bicubic
			{
				{

					HorizontalBicubic<float>(pPlane[2], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
					VerticalBicubic<float>(pPlane[8], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
					HorizontalBicubic<float>(pPlane[10], pPlane[8], nPitch, nPitch, nExtendedWidth, nExtendedHeight); // faster from ready-made horizontal

				}
			}
			else // Wiener
			{

				HorizontalWiener<float>(pPlane[2], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				VerticalWiener<float>(pPlane[8], pPlane[0], nPitch, nPitch, nExtendedWidth, nExtendedHeight);
				HorizontalWiener<float>(pPlane[10], pPlane[8], nPitch, nPitch, nExtendedWidth, nExtendedHeight); // faster from ready-made horizontal

			}
			// now interpolate intermediate

			Average2<float>(pPlane[1], pPlane[0], pPlane[2], nPitch, nExtendedWidth, nExtendedHeight);
			Average2<float>(pPlane[9], pPlane[8], pPlane[10], nPitch, nExtendedWidth, nExtendedHeight);
			Average2<float>(pPlane[4], pPlane[0], pPlane[8], nPitch, nExtendedWidth, nExtendedHeight);
			Average2<float>(pPlane[6], pPlane[2], pPlane[10], nPitch, nExtendedWidth, nExtendedHeight);
			Average2<float>(pPlane[5], pPlane[4], pPlane[6], nPitch, nExtendedWidth, nExtendedHeight);

			Average2<float>(pPlane[3], pPlane[0] + 4, pPlane[2], nPitch, nExtendedWidth - 1, nExtendedHeight);
			Average2<float>(pPlane[11], pPlane[8] + 4, pPlane[10], nPitch, nExtendedWidth - 1, nExtendedHeight);
			Average2<float>(pPlane[12], pPlane[0] + nPitch, pPlane[8], nPitch, nExtendedWidth, nExtendedHeight - 1);
			Average2<float>(pPlane[14], pPlane[2] + nPitch, pPlane[10], nPitch, nExtendedWidth, nExtendedHeight - 1);
			Average2<float>(pPlane[13], pPlane[12], pPlane[14], nPitch, nExtendedWidth, nExtendedHeight);
			Average2<float>(pPlane[7], pPlane[4] + 4, pPlane[6], nPitch, nExtendedWidth - 1, nExtendedHeight);
			Average2<float>(pPlane[15], pPlane[12] + 4, pPlane[14], nPitch, nExtendedWidth - 1, nExtendedHeight);


		}

		isRefined = true;
		//   LeaveCriticalSection(&cs);
	}
	void RefineExt(const uint8_t* pSrc2x, int32_t nSrc2xPitch, bool isExtPadded) {
		if ((nPel == 2) && (!isRefined))
			RefineExtPel2<float>(pSrc2x, nSrc2xPitch, isExtPadded);
		else if ((nPel == 4) && (!isRefined))
			RefineExtPel4<float>(pSrc2x, nSrc2xPitch, isExtPadded);
		
		isRefined = true;
	}
	void ReduceTo(MVPlane* pReducedPlane, int32_t rfilter) {
		//    EnterCriticalSection(&cs);
		if (!pReducedPlane->isFilled)
		{
			if (rfilter == 0)
			{
				{
					RB2F_C<float>(pReducedPlane->pPlane[0] + pReducedPlane->nOffsetPadding, pPlane[0] + nOffsetPadding,
						pReducedPlane->nPitch, nPitch, pReducedPlane->nWidth, pReducedPlane->nHeight);
				}
			}
			else if (rfilter == 1)
			{
				RB2Filtered<float>(pReducedPlane->pPlane[0] + pReducedPlane->nOffsetPadding, pPlane[0] + nOffsetPadding,
					pReducedPlane->nPitch, nPitch, pReducedPlane->nWidth, pReducedPlane->nHeight);
			}
			else if (rfilter == 2)
			{
				RB2BilinearFiltered<float>(pReducedPlane->pPlane[0] + pReducedPlane->nOffsetPadding, pPlane[0] + nOffsetPadding,
					pReducedPlane->nPitch, nPitch, pReducedPlane->nWidth, pReducedPlane->nHeight);
			}
			else if (rfilter == 3)
			{
				RB2Quadratic<float>(pReducedPlane->pPlane[0] + pReducedPlane->nOffsetPadding, pPlane[0] + nOffsetPadding,
					pReducedPlane->nPitch, nPitch, pReducedPlane->nWidth, pReducedPlane->nHeight);
			}
			else if (rfilter == 4)
			{
				RB2Cubic<float>(pReducedPlane->pPlane[0] + pReducedPlane->nOffsetPadding, pPlane[0] + nOffsetPadding,
					pReducedPlane->nPitch, nPitch, pReducedPlane->nWidth, pReducedPlane->nHeight);
			}
		}
		pReducedPlane->isFilled = true;
		//   LeaveCriticalSection(&cs);
	}
	void WritePlane(FILE* pFile) {
		for (int32_t i = 0; i < nHeight; i++)
			fwrite(pPlane[0] + i * nPitch + nOffsetPadding, 1, nWidth, pFile);
	}
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
	MVFrame(int32_t nWidth, int32_t nHeight, int32_t nPel, int32_t nHPad, int32_t nVPad, int32_t _nMode, int32_t _xRatioUV, int32_t _yRatioUV) {
		nMode = _nMode;
		xRatioUV = _xRatioUV;
		yRatioUV = _yRatioUV;

		if (nMode & YPLANE)
			pYPlane = new MVPlane(nWidth, nHeight, nPel, nHPad, nVPad);
		else
			pYPlane = 0;

		if (nMode & UPLANE)
			pUPlane = new MVPlane(nWidth / xRatioUV, nHeight / yRatioUV, nPel, nHPad / xRatioUV, nVPad / yRatioUV);
		else
			pUPlane = 0;

		if (nMode & VPLANE)
			pVPlane = new MVPlane(nWidth / xRatioUV, nHeight / yRatioUV, nPel, nHPad / xRatioUV, nVPad / yRatioUV);
		else
			pVPlane = 0;
	}
	~MVFrame() {
		if (nMode & YPLANE)
			delete pYPlane;

		if (nMode & UPLANE)
			delete pUPlane;

		if (nMode & VPLANE)
			delete pVPlane;
	}
	void Update(int32_t _nMode, uint8_t* pSrcY, int32_t pitchY, uint8_t* pSrcU, int32_t pitchU, uint8_t* pSrcV, int32_t pitchV) {
		if (_nMode & nMode & YPLANE) //v2.0.8
			pYPlane->Update(pSrcY, pitchY);

		if (_nMode & nMode & UPLANE)
			pUPlane->Update(pSrcU, pitchU);

		if (_nMode & nMode & VPLANE)
			pVPlane->Update(pSrcV, pitchV);
	}
	void ChangePlane(const uint8_t* pNewPlane, int32_t nNewPitch, MVPlaneSet _nMode) {
		if (_nMode & nMode & YPLANE)
			pYPlane->ChangePlane(pNewPlane, nNewPitch);

		if (_nMode & nMode & UPLANE)
			pUPlane->ChangePlane(pNewPlane, nNewPitch);

		if (_nMode & nMode & VPLANE)
			pVPlane->ChangePlane(pNewPlane, nNewPitch);
	}
	void Refine(MVPlaneSet _nMode, int32_t sharp) {
		if (nMode & YPLANE & _nMode)
			pYPlane->Refine(sharp);

		if (nMode & UPLANE & _nMode)
			pUPlane->Refine(sharp);

		if (nMode & VPLANE & _nMode)
			pVPlane->Refine(sharp);
	}
	void Pad(MVPlaneSet _nMode) {
		if (nMode & YPLANE & _nMode)
			pYPlane->Pad();

		if (nMode & UPLANE & _nMode)
			pUPlane->Pad();

		if (nMode & VPLANE & _nMode)
			pVPlane->Pad();
	}
	void ReduceTo(MVFrame* pFrame, MVPlaneSet _nMode, int32_t rfilter) {
		if (nMode & YPLANE & _nMode)
			pYPlane->ReduceTo(pFrame->GetPlane(YPLANE), rfilter);

		if (nMode & UPLANE & _nMode)
			pUPlane->ReduceTo(pFrame->GetPlane(UPLANE), rfilter);

		if (nMode & VPLANE & _nMode)
			pVPlane->ReduceTo(pFrame->GetPlane(VPLANE), rfilter);
	}
	void ResetState() {
		if (nMode & YPLANE)
			pYPlane->ResetState();

		if (nMode & UPLANE)
			pUPlane->ResetState();

		if (nMode & VPLANE)
			pVPlane->ResetState();
	}
	void WriteFrame(FILE* pFile) {
		if (nMode & YPLANE)
			pYPlane->WritePlane(pFile);

		if (nMode & UPLANE)
			pUPlane->WritePlane(pFile);

		if (nMode & VPLANE)
			pVPlane->WritePlane(pFile);
	}
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
	MVGroupOfFrames(int32_t _nLevelCount, int32_t _nWidth, int32_t _nHeight, int32_t _nPel, int32_t _nHPad, int32_t _nVPad, int32_t nMode, int32_t _xRatioUV, int32_t _yRatioUV) {
		nLevelCount = _nLevelCount;
		nWidth = _nWidth;
		nHeight = _nHeight;
		nPel = _nPel;
		nHPad = _nHPad;
		nVPad = _nVPad;
		xRatioUV = _xRatioUV;
		yRatioUV = _yRatioUV;
		pFrames = new MVFrame * [nLevelCount];

		pFrames[0] = new MVFrame(nWidth, nHeight, nPel, nHPad, nVPad, nMode, xRatioUV, yRatioUV);
		for (int32_t i = 1; i < nLevelCount; i++)
		{
			int32_t nWidthi = PlaneWidthLuma(nWidth, i, xRatioUV, nHPad);//(nWidthi / 2) - ((nWidthi / 2) % xRatioUV); //  even for YV12
			int32_t nHeighti = PlaneHeightLuma(nHeight, i, yRatioUV, nVPad);//(nHeighti / 2) - ((nHeighti / 2) % yRatioUV); // even for YV12
			pFrames[i] = new MVFrame(nWidthi, nHeighti, 1, nHPad, nVPad, nMode, xRatioUV, yRatioUV);
		}
	}
	~MVGroupOfFrames() {
		for (int32_t i = 0; i < nLevelCount; i++)
			delete pFrames[i];

		delete[] pFrames;
	}
	void Update(int32_t nMode, uint8_t* pSrcY, int32_t pitchY, uint8_t* pSrcU, int32_t pitchU, uint8_t* pSrcV, int32_t pitchV) {
		for (int32_t i = 0; i < nLevelCount; i++)
		{
			uint32_t offY = PlaneSuperOffset(false, nHeight, i, nPel, nVPad, pitchY, yRatioUV);
			uint32_t offU = PlaneSuperOffset(true, nHeight / yRatioUV, i, nPel, nVPad / yRatioUV, pitchU, yRatioUV);
			uint32_t offV = PlaneSuperOffset(true, nHeight / yRatioUV, i, nPel, nVPad / yRatioUV, pitchV, yRatioUV);
			pFrames[i]->Update(nMode, pSrcY + offY, pitchY, pSrcU + offU, pitchU, pSrcV + offV, pitchV);
		}
	}
	MVFrame* GetFrame(int32_t nLevel) {
		if ((nLevel < 0) || (nLevel >= nLevelCount)) return 0;
		return pFrames[nLevel];
	}
	void SetPlane(const uint8_t* pNewSrc, int32_t nNewPitch, MVPlaneSet nMode) {
		pFrames[0]->ChangePlane(pNewSrc, nNewPitch, nMode);
	}
	void Refine(MVPlaneSet nMode, int32_t sharp) {
		pFrames[0]->Refine(nMode, sharp);
	}
	void Pad(MVPlaneSet nMode) {
		pFrames[0]->Pad(nMode);
	}
	void Reduce(MVPlaneSet _nMode, int32_t rfilter) {
		for (int32_t i = 0; i < nLevelCount - 1; i++)
		{
			pFrames[i]->ReduceTo(pFrames[i + 1], _nMode, rfilter);
			pFrames[i + 1]->Pad(YUVPLANES);
		}
	}
	void ResetState() {
		for (int32_t i = 0; i < nLevelCount; i++)
			pFrames[i]->ResetState();
	}
};
