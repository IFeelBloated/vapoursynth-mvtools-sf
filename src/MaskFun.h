#ifndef __MASKFUN__
#define __MASKFUN__
#include <cstdint>
#include "MVClip.h"
#include "MVFrame.h"

void Merge4PlanesToBig(uint8_t *pel2Plane, int32_t pel2Pitch, const uint8_t *pPlane0, const uint8_t *pPlane1,
	const uint8_t *pPlane2, const uint8_t * pPlane3, int32_t width, int32_t height, int32_t pitch);
void Merge16PlanesToBig(uint8_t *pel4Plane, int32_t pel4Pitch,
	const uint8_t *pPlane0, const uint8_t *pPlane1, const uint8_t *pPlane2, const uint8_t * pPlane3,
	const uint8_t *pPlane4, const uint8_t *pPlane5, const uint8_t *pPlane6, const uint8_t * pPlane7,
	const uint8_t *pPlane8, const uint8_t *pPlane9, const uint8_t *pPlane10, const uint8_t * pPlane11,
	const uint8_t *pPlane12, const uint8_t * pPlane13, const uint8_t *pPlane14, const uint8_t * pPlane15,
	int32_t width, int32_t height, int32_t pitch);
void MakeVectorSmallMasks(MVClipBalls *mvClip, int32_t nX, int32_t nY, uint8_t *VXSmallY, int32_t pitchVXSmallY, uint8_t *VYSmallY, int32_t pitchVYSmallY);
void VectorSmallMaskYToHalfUV(uint8_t * VSmallY, int32_t nBlkX, int32_t nBlkY, uint8_t *VSmallUV, int32_t ratioUV);
void Blend(uint8_t * pdst, const uint8_t * psrc, const uint8_t * pref, int32_t height, int32_t width, int32_t dst_pitch, int32_t src_pitch, int32_t ref_pitch, int32_t time256);
void MakeVectorOcclusionMaskTime(MVClipBalls *mvClip, int32_t nBlkX, int32_t nBlkY, double dMaskNormFactor, double fGamma, int32_t nPel, uint8_t * occMask, int32_t occMaskPitch, int32_t time256, int32_t blkSizeX, int32_t blkSizeY);
void FlowInterExtra(uint8_t * pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, const int32_t *LUTVB, const int32_t * LUTVF,
	uint8_t *VXFullBB, uint8_t *VXFullFF, uint8_t *VYFullBB, uint8_t *VYFullFF);
void FlowInter(uint8_t * pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, const int32_t *LUTVB, const int32_t *LUTVF);
void Create_LUTV(int32_t time256, int32_t *LUTVB, int32_t *LUTVF);
void FlowInterSimple(uint8_t * pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, int32_t *LUTVB, int32_t *LUTVF);

#endif
