#pragma once
#include <cmath>
#include <cstdint>

constexpr auto MV_PI = 3.1415926535897932384626433832795;
constexpr auto OW_TL = 0;
constexpr auto OW_TM = 1;
constexpr auto OW_TR = 2;
constexpr auto OW_ML = 3;
constexpr auto OW_MM = 4;
constexpr auto OW_MR = 5;
constexpr auto OW_BL = 6;
constexpr auto OW_BM = 7;
constexpr auto OW_BR = 8;

class OverlapWindows {
	int32_t nx;
	int32_t ny;
	int32_t ox;
	int32_t oy;
	int32_t size;
	double *Overlap9Windows;
	double *fWin1UVx;
	double *fWin1UVxfirst;
	double *fWin1UVxlast;
	double *fWin1UVy;
	double *fWin1UVyfirst;
	double *fWin1UVylast;
public:
	OverlapWindows(int32_t _nx, int32_t _ny, int32_t _ox, int32_t _oy) {
		nx = _nx;
		ny = _ny;
		ox = _ox;
		oy = _oy;
		size = nx * ny;
		fWin1UVx = new double[nx];
		fWin1UVxfirst = new double[nx];
		fWin1UVxlast = new double[nx];
		for (int32_t i = 0; i < ox; i++)
		{
			fWin1UVx[i] = cos(MV_PI * (i - ox + 0.5f) / (ox * 2));
			fWin1UVx[i] = fWin1UVx[i] * fWin1UVx[i];// left window (rised cosine)
			fWin1UVxfirst[i] = 1; // very first window
			fWin1UVxlast[i] = fWin1UVx[i]; // very last
		}
		for (int32_t i = ox; i < nx - ox; i++)
		{
			fWin1UVx[i] = 1;
			fWin1UVxfirst[i] = 1; // very first window
			fWin1UVxlast[i] = 1; // very last
		}
		for (int32_t i = nx - ox; i < nx; i++)
		{
			fWin1UVx[i] = cos(MV_PI * (i - nx + ox + 0.5f) / (ox * 2));
			fWin1UVx[i] = fWin1UVx[i] * fWin1UVx[i];// right window (falled cosine)
			fWin1UVxfirst[i] = fWin1UVx[i]; // very first window
			fWin1UVxlast[i] = 1; // very last
		}

		fWin1UVy = new double[ny];
		fWin1UVyfirst = new double[ny];
		fWin1UVylast = new double[ny];
		for (int32_t i = 0; i < oy; i++)
		{
			fWin1UVy[i] = cos(MV_PI * (i - oy + 0.5f) / (oy * 2));
			fWin1UVy[i] = fWin1UVy[i] * fWin1UVy[i];// left window (rised cosine)
			fWin1UVyfirst[i] = 1; // very first window
			fWin1UVylast[i] = fWin1UVy[i]; // very last
		}
		for (int32_t i = oy; i < ny - oy; i++)
		{
			fWin1UVy[i] = 1;
			fWin1UVyfirst[i] = 1; // very first window
			fWin1UVylast[i] = 1; // very last
		}
		for (int32_t i = ny - oy; i < ny; i++)
		{
			fWin1UVy[i] = cos(MV_PI * (i - ny + oy + 0.5f) / (oy * 2));
			fWin1UVy[i] = fWin1UVy[i] * fWin1UVy[i];// right window (falled cosine)
			fWin1UVyfirst[i] = fWin1UVy[i]; // very first window
			fWin1UVylast[i] = 1; // very last
		}


		Overlap9Windows = new double[size * 9];

		auto winOverUVTL = Overlap9Windows;
		auto winOverUVTM = Overlap9Windows + size;
		auto winOverUVTR = Overlap9Windows + size * 2;
		auto winOverUVML = Overlap9Windows + size * 3;
		auto winOverUVMM = Overlap9Windows + size * 4;
		auto winOverUVMR = Overlap9Windows + size * 5;
		auto winOverUVBL = Overlap9Windows + size * 6;
		auto winOverUVBM = Overlap9Windows + size * 7;
		auto winOverUVBR = Overlap9Windows + size * 8;

		for (int32_t j = 0; j < ny; j++)
		{
			for (int32_t i = 0; i < nx; i++)
			{
				winOverUVTL[i] = fWin1UVyfirst[j] * fWin1UVxfirst[i] * 2048;
				winOverUVTM[i] = fWin1UVyfirst[j] * fWin1UVx[i] * 2048;
				winOverUVTR[i] = fWin1UVyfirst[j] * fWin1UVxlast[i] * 2048;
				winOverUVML[i] = fWin1UVy[j] * fWin1UVxfirst[i] * 2048;
				winOverUVMM[i] = fWin1UVy[j] * fWin1UVx[i] * 2048;
				winOverUVMR[i] = fWin1UVy[j] * fWin1UVxlast[i] * 2048;
				winOverUVBL[i] = fWin1UVylast[j] * fWin1UVxfirst[i] * 2048;
				winOverUVBM[i] = fWin1UVylast[j] * fWin1UVx[i] * 2048;
				winOverUVBR[i] = fWin1UVylast[j] * fWin1UVxlast[i] * 2048;
			}
			winOverUVTL += nx;
			winOverUVTM += nx;
			winOverUVTR += nx;
			winOverUVML += nx;
			winOverUVMM += nx;
			winOverUVMR += nx;
			winOverUVBL += nx;
			winOverUVBM += nx;
			winOverUVBR += nx;
		}
	}
	~OverlapWindows() {
		delete[] Overlap9Windows;
		delete[] fWin1UVx;
		delete[] fWin1UVxfirst;
		delete[] fWin1UVxlast;
		delete[] fWin1UVy;
		delete[] fWin1UVyfirst;
		delete[] fWin1UVylast;
	}
	inline int32_t Getnx() const { return nx; }
	inline int32_t Getny() const { return ny; }
	inline int32_t GetSize() const { return size; }
	auto GetWindow(int32_t i) const { return Overlap9Windows + size*i; }
};

using OverlapsFunction = auto(*)(uint8_t *pDst, intptr_t nDstPitch,
	const uint8_t *pSrc, intptr_t nSrcPitch,
	double *pWin, intptr_t nWinPitch)->void;

template <int32_t blockWidth, int32_t blockHeight, typename PixelType2, typename PixelType>
void Overlaps_C(uint8_t *pDst8, intptr_t nDstPitch, const uint8_t *pSrc8, intptr_t nSrcPitch, double *pWin, intptr_t nWinPitch) {
	for (int32_t j = 0; j<blockHeight; j++) {
		for (int32_t i = 0; i<blockWidth; i++) {
			PixelType2 *pDst = (PixelType2 *)pDst8;
			const PixelType *pSrc = (const PixelType *)pSrc8;
			pDst[i] += (static_cast<PixelType2>(pSrc[i]) * pWin[i]) / 64.;
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
		pWin += nWinPitch;
	}
}

using ToPixelsFunction = auto(*)(uint8_t *pDst, int32_t nDstPitch,
	const uint8_t *pSrc, int32_t nSrcPitch,
	int32_t width, int32_t height)->void;

template <typename PixelType2, typename PixelType>
void ToPixels(uint8_t *pDst8, int32_t nDstPitch, const uint8_t *pSrc8, int32_t nSrcPitch, int32_t nWidth, int32_t nHeight) {
	for (int32_t h = 0; h<nHeight; h++) {
		for (int32_t i = 0; i<nWidth; i++) {
			const PixelType2 *pSrc = (const PixelType2 *)pSrc8;
			PixelType *pDst = (PixelType *)pDst8;
			pDst[i] = static_cast<PixelType>(pSrc[i] / 32.);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
	}
}

auto CosineAnnealing(auto StartPoint, auto EndPoint, auto Position, auto Radius) {
	auto x = (Position - 1) * MV_PI / (Radius - 1);
	auto Ratio = (1. - cos(x)) * 0.5;
	return StartPoint + Ratio * (EndPoint - StartPoint);
}