// Create an overlay mask with the motion vectors

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

#ifndef __MASKFUN__
#define __MASKFUN__

#include <stdint.h>

#include "MVClip.h"
#include "MVFrame.h"

void Merge4PlanesToBig(uint8_t *pel2Plane, int pel2Pitch, const uint8_t *pPlane0, const uint8_t *pPlane1,
	const uint8_t *pPlane2, const uint8_t * pPlane3, int width, int height, int pitch);
void Merge16PlanesToBig(uint8_t *pel4Plane, int pel4Pitch,
	const uint8_t *pPlane0, const uint8_t *pPlane1, const uint8_t *pPlane2, const uint8_t * pPlane3,
	const uint8_t *pPlane4, const uint8_t *pPlane5, const uint8_t *pPlane6, const uint8_t * pPlane7,
	const uint8_t *pPlane8, const uint8_t *pPlane9, const uint8_t *pPlane10, const uint8_t * pPlane11,
	const uint8_t *pPlane12, const uint8_t * pPlane13, const uint8_t *pPlane14, const uint8_t * pPlane15,
	int width, int height, int pitch);
void MakeVectorSmallMasks(MVClipBalls *mvClip, int nX, int nY, uint8_t *VXSmallY, int pitchVXSmallY, uint8_t *VYSmallY, int pitchVYSmallY);
void VectorSmallMaskYToHalfUV(uint8_t * VSmallY, int nBlkX, int nBlkY, uint8_t *VSmallUV, int ratioUV);
void Blend(uint8_t * pdst, const uint8_t * psrc, const uint8_t * pref, int height, int width, int dst_pitch, int src_pitch, int ref_pitch, int time256);
#endif
