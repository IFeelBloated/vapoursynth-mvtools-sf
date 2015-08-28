// Create an overlay mask with the motion vectors
// Copyright(c)2006 A.G.Balakhnin aka Fizick
// See legal notice in Copying.txt for more information

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .

#include <cstring>
#include <math.h>
#include <stdint.h>

#include "MaskFun.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) > (b) ? (b) : (a))

template <typename PixelType>
void RealMerge4PlanesToBig(uint8_t *pel2Plane_u8, int pel2Pitch, const uint8_t *pPlane0_u8, const uint8_t *pPlane1_u8,
	const uint8_t *pPlane2_u8, const uint8_t * pPlane3_u8, int width, int height, int pitch)
{
		for (int h = 0; h<height; h++)
		{
			for (int w = 0; w<width; w++)
			{
				PixelType *pel2Plane = (PixelType *)pel2Plane_u8;
				const PixelType *pPlane0 = (const PixelType *)pPlane0_u8;
				const PixelType *pPlane1 = (const PixelType *)pPlane1_u8;

				pel2Plane[w << 1] = pPlane0[w];
				pel2Plane[(w << 1) + 1] = pPlane1[w];
			}
			pel2Plane_u8 += pel2Pitch;
			for (int w = 0; w<width; w++)
			{
				PixelType *pel2Plane = (PixelType *)pel2Plane_u8;
				const PixelType *pPlane2 = (const PixelType *)pPlane2_u8;
				const PixelType *pPlane3 = (const PixelType *)pPlane3_u8;

				pel2Plane[w << 1] = pPlane2[w];
				pel2Plane[(w << 1) + 1] = pPlane3[w];
			}
			pel2Plane_u8 += pel2Pitch;
			pPlane0_u8 += pitch;
			pPlane1_u8 += pitch;
			pPlane2_u8 += pitch;
			pPlane3_u8 += pitch;
		}
}

void Merge4PlanesToBig(uint8_t *pel2Plane, int pel2Pitch, const uint8_t *pPlane0, const uint8_t *pPlane1, const uint8_t *pPlane2, const uint8_t *pPlane3, int width, int height, int pitch) {
		RealMerge4PlanesToBig<float>(pel2Plane, pel2Pitch, pPlane0, pPlane1, pPlane2, pPlane3, width, height, pitch);
}

template <typename PixelType>
void RealMerge16PlanesToBig(uint8_t *pel4Plane_u8, int pel4Pitch,
	const uint8_t *pPlane0_u8, const uint8_t *pPlane1_u8, const uint8_t *pPlane2_u8, const uint8_t * pPlane3_u8,
	const uint8_t *pPlane4_u8, const uint8_t *pPlane5_u8, const uint8_t *pPlane6_u8, const uint8_t * pPlane7_u8,
	const uint8_t *pPlane8_u8, const uint8_t *pPlane9_u8, const uint8_t *pPlane10_u8, const uint8_t * pPlane11_u8,
	const uint8_t *pPlane12_u8, const uint8_t * pPlane13_u8, const uint8_t *pPlane14_u8, const uint8_t * pPlane15_u8,
	int width, int height, int pitch)
{
	{
		for (int h = 0; h<height; h++)
		{
			for (int w = 0; w<width; w++)
			{
				PixelType *pel4Plane = (PixelType *)pel4Plane_u8;
				const PixelType *pPlane0 = (const PixelType *)pPlane0_u8;
				const PixelType *pPlane1 = (const PixelType *)pPlane1_u8;
				const PixelType *pPlane2 = (const PixelType *)pPlane2_u8;
				const PixelType *pPlane3 = (const PixelType *)pPlane3_u8;

				pel4Plane[w << 2] = pPlane0[w];
				pel4Plane[(w << 2) + 1] = pPlane1[w];
				pel4Plane[(w << 2) + 2] = pPlane2[w];
				pel4Plane[(w << 2) + 3] = pPlane3[w];
			}
			pel4Plane_u8 += pel4Pitch;
			for (int w = 0; w<width; w++)
			{
				PixelType *pel4Plane = (PixelType *)pel4Plane_u8;
				const PixelType *pPlane4 = (const PixelType *)pPlane4_u8;
				const PixelType *pPlane5 = (const PixelType *)pPlane5_u8;
				const PixelType *pPlane6 = (const PixelType *)pPlane6_u8;
				const PixelType *pPlane7 = (const PixelType *)pPlane7_u8;

				pel4Plane[w << 2] = pPlane4[w];
				pel4Plane[(w << 2) + 1] = pPlane5[w];
				pel4Plane[(w << 2) + 2] = pPlane6[w];
				pel4Plane[(w << 2) + 3] = pPlane7[w];
			}
			pel4Plane_u8 += pel4Pitch;
			for (int w = 0; w<width; w++)
			{
				PixelType *pel4Plane = (PixelType *)pel4Plane_u8;
				const PixelType *pPlane8 = (const PixelType *)pPlane8_u8;
				const PixelType *pPlane9 = (const PixelType *)pPlane9_u8;
				const PixelType *pPlane10 = (const PixelType *)pPlane10_u8;
				const PixelType *pPlane11 = (const PixelType *)pPlane11_u8;

				pel4Plane[w << 2] = pPlane8[w];
				pel4Plane[(w << 2) + 1] = pPlane9[w];
				pel4Plane[(w << 2) + 2] = pPlane10[w];
				pel4Plane[(w << 2) + 3] = pPlane11[w];
			}
			pel4Plane_u8 += pel4Pitch;
			for (int w = 0; w<width; w++)
			{
				PixelType *pel4Plane = (PixelType *)pel4Plane_u8;
				const PixelType *pPlane12 = (const PixelType *)pPlane12_u8;
				const PixelType *pPlane13 = (const PixelType *)pPlane13_u8;
				const PixelType *pPlane14 = (const PixelType *)pPlane14_u8;
				const PixelType *pPlane15 = (const PixelType *)pPlane15_u8;

				pel4Plane[w << 2] = pPlane12[w];
				pel4Plane[(w << 2) + 1] = pPlane13[w];
				pel4Plane[(w << 2) + 2] = pPlane14[w];
				pel4Plane[(w << 2) + 3] = pPlane15[w];
			}
			pel4Plane_u8 += pel4Pitch;
			pPlane0_u8 += pitch;
			pPlane1_u8 += pitch;
			pPlane2_u8 += pitch;
			pPlane3_u8 += pitch;
			pPlane4_u8 += pitch;
			pPlane5_u8 += pitch;
			pPlane6_u8 += pitch;
			pPlane7_u8 += pitch;
			pPlane8_u8 += pitch;
			pPlane9_u8 += pitch;
			pPlane10_u8 += pitch;
			pPlane11_u8 += pitch;
			pPlane12_u8 += pitch;
			pPlane13_u8 += pitch;
			pPlane14_u8 += pitch;
			pPlane15_u8 += pitch;
		}
	}
}

void Merge16PlanesToBig(uint8_t *pel4Plane, int pel4Pitch,
	const uint8_t *pPlane0, const uint8_t *pPlane1, const uint8_t *pPlane2, const uint8_t * pPlane3,
	const uint8_t *pPlane4, const uint8_t *pPlane5, const uint8_t *pPlane6, const uint8_t * pPlane7,
	const uint8_t *pPlane8, const uint8_t *pPlane9, const uint8_t *pPlane10, const uint8_t * pPlane11,
	const uint8_t *pPlane12, const uint8_t * pPlane13, const uint8_t *pPlane14, const uint8_t * pPlane15,
	int width, int height, int pitch) {
		RealMerge16PlanesToBig<float>(pel4Plane, pel4Pitch, pPlane0, pPlane1, pPlane2, pPlane3, pPlane4, pPlane5, pPlane6, pPlane7, pPlane8, pPlane9, pPlane10, pPlane11, pPlane12, pPlane13, pPlane14, pPlane15, width, height, pitch);
}
