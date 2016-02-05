#include "Overlap.h"

OverlapWindows::OverlapWindows(int32_t _nx, int32_t _ny, int32_t _ox, int32_t _oy)
{
	nx = _nx;
	ny = _ny;
	ox = _ox;
	oy = _oy;
	size = nx*ny;
	fWin1UVx = new float[nx];
	fWin1UVxfirst = new float[nx];
	fWin1UVxlast = new float[nx];
	for (int32_t i = 0; i<ox; i++)
	{
		fWin1UVx[i] = static_cast <float> (cos(M_PI*(i - ox + 0.5f) / (ox * 2)));
		fWin1UVx[i] = fWin1UVx[i] * fWin1UVx[i];// left window (rised cosine)
		fWin1UVxfirst[i] = 1; // very first window
		fWin1UVxlast[i] = fWin1UVx[i]; // very last
	}
	for (int32_t i = ox; i<nx - ox; i++)
	{
		fWin1UVx[i] = 1;
		fWin1UVxfirst[i] = 1; // very first window
		fWin1UVxlast[i] = 1; // very last
	}
	for (int32_t i = nx - ox; i<nx; i++)
	{
		fWin1UVx[i] = static_cast <float> (cos(M_PI*(i - nx + ox + 0.5f) / (ox * 2)));
		fWin1UVx[i] = fWin1UVx[i] * fWin1UVx[i];// right window (falled cosine)
		fWin1UVxfirst[i] = fWin1UVx[i]; // very first window
		fWin1UVxlast[i] = 1; // very last
	}

	fWin1UVy = new float[ny];
	fWin1UVyfirst = new float[ny];
	fWin1UVylast = new float[ny];
	for (int32_t i = 0; i<oy; i++)
	{
		fWin1UVy[i] = static_cast <float> (cos(M_PI*(i - oy + 0.5f) / (oy * 2)));
		fWin1UVy[i] = fWin1UVy[i] * fWin1UVy[i];// left window (rised cosine)
		fWin1UVyfirst[i] = 1; // very first window
		fWin1UVylast[i] = fWin1UVy[i]; // very last
	}
	for (int32_t i = oy; i<ny - oy; i++)
	{
		fWin1UVy[i] = 1;
		fWin1UVyfirst[i] = 1; // very first window
		fWin1UVylast[i] = 1; // very last
	}
	for (int32_t i = ny - oy; i<ny; i++)
	{
		fWin1UVy[i] = static_cast <float> (cos(M_PI*(i - ny + oy + 0.5f) / (oy * 2)));
		fWin1UVy[i] = fWin1UVy[i] * fWin1UVy[i];// right window (falled cosine)
		fWin1UVyfirst[i] = fWin1UVy[i]; // very first window
		fWin1UVylast[i] = 1; // very last
	}


	Overlap9Windows = new int16_t[size * 9];

	int16_t *winOverUVTL = Overlap9Windows;
	int16_t *winOverUVTM = Overlap9Windows + size;
	int16_t *winOverUVTR = Overlap9Windows + size * 2;
	int16_t *winOverUVML = Overlap9Windows + size * 3;
	int16_t *winOverUVMM = Overlap9Windows + size * 4;
	int16_t *winOverUVMR = Overlap9Windows + size * 5;
	int16_t *winOverUVBL = Overlap9Windows + size * 6;
	int16_t *winOverUVBM = Overlap9Windows + size * 7;
	int16_t *winOverUVBR = Overlap9Windows + size * 8;

	for (int32_t j = 0; j<ny; j++)
	{
		for (int32_t i = 0; i<nx; i++)
		{
			winOverUVTL[i] = (int32_t)(fWin1UVyfirst[j] * fWin1UVxfirst[i] * 2048 + 0.5f);
			winOverUVTM[i] = (int32_t)(fWin1UVyfirst[j] * fWin1UVx[i] * 2048 + 0.5f);
			winOverUVTR[i] = (int32_t)(fWin1UVyfirst[j] * fWin1UVxlast[i] * 2048 + 0.5f);
			winOverUVML[i] = (int32_t)(fWin1UVy[j] * fWin1UVxfirst[i] * 2048 + 0.5f);
			winOverUVMM[i] = (int32_t)(fWin1UVy[j] * fWin1UVx[i] * 2048 + 0.5f);
			winOverUVMR[i] = (int32_t)(fWin1UVy[j] * fWin1UVxlast[i] * 2048 + 0.5f);
			winOverUVBL[i] = (int32_t)(fWin1UVylast[j] * fWin1UVxfirst[i] * 2048 + 0.5f);
			winOverUVBM[i] = (int32_t)(fWin1UVylast[j] * fWin1UVx[i] * 2048 + 0.5f);
			winOverUVBR[i] = (int32_t)(fWin1UVylast[j] * fWin1UVxlast[i] * 2048 + 0.5f);
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

OverlapWindows::~OverlapWindows()
{
	delete[] Overlap9Windows;
	delete[] fWin1UVx;
	delete[] fWin1UVxfirst;
	delete[] fWin1UVxlast;
	delete[] fWin1UVy;
	delete[] fWin1UVyfirst;
	delete[] fWin1UVylast;
}
