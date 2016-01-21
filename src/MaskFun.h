#ifndef __MASKFUN__
#define __MASKFUN__
#include <cstdint>
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
void MakeVectorOcclusionMaskTime(MVClipBalls *mvClip, int nBlkX, int nBlkY, double dMaskNormFactor, double fGamma, int nPel, uint8_t * occMask, int occMaskPitch, int time256, int blkSizeX, int blkSizeY);
void FlowInterExtra(uint8_t * pdst, int dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int VPitch, int width, int height, int time256, int nPel, const int *LUTVB, const int * LUTVF,
	uint8_t *VXFullBB, uint8_t *VXFullFF, uint8_t *VYFullBB, uint8_t *VYFullFF);
void FlowInter(uint8_t * pdst, int dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int VPitch, int width, int height, int time256, int nPel, const int *LUTVB, const int *LUTVF);
void Create_LUTV(int time256, int *LUTVB, int *LUTVF);
void FlowInterSimple(uint8_t * pdst, int dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int VPitch, int width, int height, int time256, int nPel, int *LUTVB, int *LUTVF);

#endif
