#include "VSHelper.h"
#include "MVFrame.h"
#include "Padding.h"
#include "Interpolation.h"
#include "MVSuper.h"

MVPlane::MVPlane(int32_t _nWidth, int32_t _nHeight, int32_t _nPel, int32_t _nHPad, int32_t _nVPad) {
	nWidth = _nWidth;
	nHeight = _nHeight;
	nPel = _nPel;
	nHPadding = _nHPad;
	nVPadding = _nVPad;
	nHPaddingPel = _nHPad * nPel;
	nVPaddingPel = _nVPad * nPel;

	nExtendedWidth = nWidth + 2 * nHPadding;
	nExtendedHeight = nHeight + 2 * nVPadding;
	pPlane = new uint8_t *[nPel * nPel];
}

MVPlane::~MVPlane() {
	delete[] pPlane;
}

void MVPlane::Update(uint8_t* pSrc, int32_t _nPitch) {
	nPitch = _nPitch;
	nOffsetPadding = nPitch * nVPadding + nHPadding * 4;

	for (int32_t i = 0; i < nPel * nPel; i++)
		pPlane[i] = pSrc + i*nPitch * nExtendedHeight;

	ResetState();
	//   LeaveCriticalSection(&cs);
}
void MVPlane::ChangePlane(const uint8_t *pNewPlane, int32_t nNewPitch)
{
	//    EnterCriticalSection(&cs);
	if (!isFilled)
		vs_bitblt(pPlane[0] + nOffsetPadding, nPitch, pNewPlane, nNewPitch, nWidth * 4, nHeight);
	isFilled = true;
	//   LeaveCriticalSection(&cs);
}

void MVPlane::Pad()
{
	//    EnterCriticalSection(&cs);
	if (!isPadded) {
		PadReferenceFrame<float>(pPlane[0], nPitch, nHPadding, nVPadding, nWidth, nHeight);
	}

	isPadded = true;
	//   LeaveCriticalSection(&cs);
}

void MVPlane::Refine(int32_t sharp)
{
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


template <typename PixelType>
void MVPlane::RefineExtPel2(const uint8_t *pSrc2x8, int32_t nSrc2xPitch, bool isExtPadded) {
	const PixelType *pSrc2x = (const PixelType *)pSrc2x8;
	PixelType *pp1 = (PixelType *)pPlane[1];
	PixelType *pp2 = (PixelType *)pPlane[2];
	PixelType *pp3 = (PixelType *)pPlane[3];

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
void MVPlane::RefineExtPel4(const uint8_t *pSrc2x8, int32_t nSrc2xPitch, bool isExtPadded) {
	const PixelType *pSrc2x = (const PixelType *)pSrc2x8;
	PixelType *pp[16];
	for (int32_t i = 1; i < 16; i++)
		pp[i] = (PixelType *)pPlane[i];

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


void MVPlane::RefineExt(const uint8_t *pSrc2x, int32_t nSrc2xPitch, bool isExtPadded) // copy from external upsized clip
{
	//    EnterCriticalSection(&cs);
	if ((nPel == 2) && (!isRefined))
	{
		RefineExtPel2<float>(pSrc2x, nSrc2xPitch, isExtPadded);
	}
	else if ((nPel == 4) && (!isRefined))
	{
		RefineExtPel4<float>(pSrc2x, nSrc2xPitch, isExtPadded);
	}
	isRefined = true;
	//   LeaveCriticalSection(&cs);

}

void MVPlane::ReduceTo(MVPlane *pReducedPlane, int32_t rfilter)
{
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

void MVPlane::WritePlane(FILE *pFile)
{
	for (int32_t i = 0; i < nHeight; i++)
		fwrite(pPlane[0] + i * nPitch + nOffsetPadding, 1, nWidth, pFile);
}


MVFrame::MVFrame(int32_t nWidth, int32_t nHeight, int32_t nPel, int32_t nHPad, int32_t nVPad, int32_t _nMode, int32_t _xRatioUV, int32_t _yRatioUV)
{
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

void MVFrame::Update(int32_t _nMode, uint8_t * pSrcY, int32_t pitchY, uint8_t * pSrcU, int32_t pitchU, uint8_t *pSrcV, int32_t pitchV)
{
	if (_nMode & nMode & YPLANE) //v2.0.8
		pYPlane->Update(pSrcY, pitchY);

	if (_nMode & nMode & UPLANE)
		pUPlane->Update(pSrcU, pitchU);

	if (_nMode & nMode & VPLANE)
		pVPlane->Update(pSrcV, pitchV);
}

MVFrame::~MVFrame()
{
	if (nMode & YPLANE)
		delete pYPlane;

	if (nMode & UPLANE)
		delete pUPlane;

	if (nMode & VPLANE)
		delete pVPlane;
}

void MVFrame::ChangePlane(const uint8_t *pNewPlane, int32_t nNewPitch, MVPlaneSet _nMode)
{
	if (_nMode & nMode & YPLANE)
		pYPlane->ChangePlane(pNewPlane, nNewPitch);

	if (_nMode & nMode & UPLANE)
		pUPlane->ChangePlane(pNewPlane, nNewPitch);

	if (_nMode & nMode & VPLANE)
		pVPlane->ChangePlane(pNewPlane, nNewPitch);
}

void MVFrame::Refine(MVPlaneSet _nMode, int32_t sharp)
{
	if (nMode & YPLANE & _nMode)
		pYPlane->Refine(sharp);

	if (nMode & UPLANE & _nMode)
		pUPlane->Refine(sharp);

	if (nMode & VPLANE & _nMode)
		pVPlane->Refine(sharp);
}

void MVFrame::Pad(MVPlaneSet _nMode)
{
	if (nMode & YPLANE & _nMode)
		pYPlane->Pad();

	if (nMode & UPLANE & _nMode)
		pUPlane->Pad();

	if (nMode & VPLANE & _nMode)
		pVPlane->Pad();
}

void MVFrame::ResetState()
{
	if (nMode & YPLANE)
		pYPlane->ResetState();

	if (nMode & UPLANE)
		pUPlane->ResetState();

	if (nMode & VPLANE)
		pVPlane->ResetState();
}

void MVFrame::WriteFrame(FILE *pFile)
{
	if (nMode & YPLANE)
		pYPlane->WritePlane(pFile);

	if (nMode & UPLANE)
		pUPlane->WritePlane(pFile);

	if (nMode & VPLANE)
		pVPlane->WritePlane(pFile);
}

void MVFrame::ReduceTo(MVFrame *pFrame, MVPlaneSet _nMode, int32_t rfilter)
{
	if (nMode & YPLANE & _nMode)
		pYPlane->ReduceTo(pFrame->GetPlane(YPLANE), rfilter);

	if (nMode & UPLANE & _nMode)
		pUPlane->ReduceTo(pFrame->GetPlane(UPLANE), rfilter);

	if (nMode & VPLANE & _nMode)
		pVPlane->ReduceTo(pFrame->GetPlane(VPLANE), rfilter);
}

/******************************************************************************
*                                                                             *
*  MVGroupOfFrames : manage a hierachal frame structure                       *
*                                                                             *
******************************************************************************/

MVGroupOfFrames::MVGroupOfFrames(int32_t _nLevelCount, int32_t _nWidth, int32_t _nHeight, int32_t _nPel, int32_t _nHPad, int32_t _nVPad, int32_t nMode, int32_t _xRatioUV, int32_t _yRatioUV)
{
	nLevelCount = _nLevelCount;
	nWidth = _nWidth;
	nHeight = _nHeight;
	nPel = _nPel;
	nHPad = _nHPad;
	nVPad = _nVPad;
	xRatioUV = _xRatioUV;
	yRatioUV = _yRatioUV;
	pFrames = new MVFrame *[nLevelCount];

	pFrames[0] = new MVFrame(nWidth, nHeight, nPel, nHPad, nVPad, nMode, xRatioUV, yRatioUV);
	for (int32_t i = 1; i < nLevelCount; i++)
	{
		int32_t nWidthi = PlaneWidthLuma(nWidth, i, xRatioUV, nHPad);//(nWidthi / 2) - ((nWidthi / 2) % xRatioUV); //  even for YV12
		int32_t nHeighti = PlaneHeightLuma(nHeight, i, yRatioUV, nVPad);//(nHeighti / 2) - ((nHeighti / 2) % yRatioUV); // even for YV12
		pFrames[i] = new MVFrame(nWidthi, nHeighti, 1, nHPad, nVPad, nMode, xRatioUV, yRatioUV);
	}
}

void MVGroupOfFrames::Update(int32_t nMode, uint8_t * pSrcY, int32_t pitchY, uint8_t * pSrcU, int32_t pitchU, uint8_t *pSrcV, int32_t pitchV) // v2.0
{
	for (int32_t i = 0; i < nLevelCount; i++)
	{
		uint32_t offY = PlaneSuperOffset(false, nHeight, i, nPel, nVPad, pitchY, yRatioUV);
		uint32_t offU = PlaneSuperOffset(true, nHeight / yRatioUV, i, nPel, nVPad / yRatioUV, pitchU, yRatioUV);
		uint32_t offV = PlaneSuperOffset(true, nHeight / yRatioUV, i, nPel, nVPad / yRatioUV, pitchV, yRatioUV);
		pFrames[i]->Update(nMode, pSrcY + offY, pitchY, pSrcU + offU, pitchU, pSrcV + offV, pitchV);
	}
}

MVGroupOfFrames::~MVGroupOfFrames()
{
	for (int32_t i = 0; i < nLevelCount; i++)
		delete pFrames[i];

	delete[] pFrames;
}

MVFrame *MVGroupOfFrames::GetFrame(int32_t nLevel)
{
	if ((nLevel < 0) || (nLevel >= nLevelCount)) return 0;
	return pFrames[nLevel];
}

void MVGroupOfFrames::SetPlane(const uint8_t *pNewSrc, int32_t nNewPitch, MVPlaneSet nMode)
{
	pFrames[0]->ChangePlane(pNewSrc, nNewPitch, nMode);
}

void MVGroupOfFrames::Refine(MVPlaneSet nMode, int32_t sharp)
{
	pFrames[0]->Refine(nMode, sharp);
}

void MVGroupOfFrames::Pad(MVPlaneSet nMode)
{
	pFrames[0]->Pad(nMode);
}

void MVGroupOfFrames::Reduce(MVPlaneSet _nMode, int32_t rfilter)
{
	for (int32_t i = 0; i < nLevelCount - 1; i++)
	{
		pFrames[i]->ReduceTo(pFrames[i + 1], _nMode, rfilter);
		pFrames[i + 1]->Pad(YUVPLANES);
	}
}

void MVGroupOfFrames::ResetState()
{
	for (int32_t i = 0; i < nLevelCount; i++)
		pFrames[i]->ResetState();
}
