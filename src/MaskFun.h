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
void MakeVectorSmallMasks(MVClipBalls *mvClip, int32_t nX, int32_t nY, int32_t *VXSmallY, int32_t pitchVXSmallY, int32_t *VYSmallY, int32_t pitchVYSmallY);
void VectorSmallMaskYToHalfUV(int32_t * VSmallY, int32_t nBlkX, int32_t nBlkY, int32_t *VSmallUV, int32_t ratioUV);
void Blend(uint8_t * pdst, const uint8_t * psrc, const uint8_t * pref, int32_t height, int32_t width, int32_t dst_pitch, int32_t src_pitch, int32_t ref_pitch, int32_t time256);
void MakeVectorOcclusionMaskTime(MVClipBalls *mvClip, bool isb, int32_t nBlkX, int32_t nBlkY, double dMaskNormDivider, double fGamma, int32_t nPel, double *occMask, int32_t occMaskPitch, int32_t time256, int32_t nBlkStepX, int32_t nBlkStepY);
void MakeSADMaskTime(MVClipBalls *mvClip, int32_t nBlkX, int32_t nBlkY, double dSADNormFactor, double fGamma, int32_t nPel, double *Mask, int32_t MaskPitch, int32_t time256, int32_t nBlkStepX, int32_t nBlkStepY);
void FlowInterExtra(uint8_t *pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	int32_t *VXFullB, int32_t *VXFullF, int32_t *VYFullB, int32_t *VYFullF, double *MaskB, double *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel,
	int32_t *VXFullBB, int32_t *VXFullFF, int32_t *VYFullBB, int32_t *VYFullFF);
void FlowInter(uint8_t *pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	int32_t *VXFullB, int32_t *VXFullF, int32_t *VYFullB, int32_t *VYFullF, double *MaskB, double *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel);
void FlowInterSimple(uint8_t *pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	int32_t *VXFullB, int32_t *VXFullF, int32_t *VYFullB, int32_t *VYFullF, double *MaskB, double *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel);

#endif
