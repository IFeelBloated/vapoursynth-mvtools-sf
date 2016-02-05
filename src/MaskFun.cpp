#include <cstring>
#include <cmath>
#include <cstdint>
#include "MaskFun.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) > (b) ? (b) : (a))

template <typename PixelType>
void RealMerge4PlanesToBig(uint8_t *pel2Plane_u8, int32_t pel2Pitch, const uint8_t *pPlane0_u8, const uint8_t *pPlane1_u8,
	const uint8_t *pPlane2_u8, const uint8_t * pPlane3_u8, int32_t width, int32_t height, int32_t pitch) {
	for (int32_t h = 0; h<height; h++) {
		for (int32_t w = 0; w<width; w++) {
			PixelType *pel2Plane = (PixelType *)pel2Plane_u8;
			const PixelType *pPlane0 = (const PixelType *)pPlane0_u8;
			const PixelType *pPlane1 = (const PixelType *)pPlane1_u8;
			pel2Plane[w << 1] = pPlane0[w];
			pel2Plane[(w << 1) + 1] = pPlane1[w];
		}
		pel2Plane_u8 += pel2Pitch;
		for (int32_t w = 0; w<width; w++) {
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

void Merge4PlanesToBig(uint8_t *pel2Plane, int32_t pel2Pitch, const uint8_t *pPlane0, const uint8_t *pPlane1, const uint8_t *pPlane2, const uint8_t *pPlane3, int32_t width, int32_t height, int32_t pitch) {
	RealMerge4PlanesToBig<float>(pel2Plane, pel2Pitch, pPlane0, pPlane1, pPlane2, pPlane3, width, height, pitch);
}

template <typename PixelType>
void RealMerge16PlanesToBig(uint8_t *pel4Plane_u8, int32_t pel4Pitch,
	const uint8_t *pPlane0_u8, const uint8_t *pPlane1_u8, const uint8_t *pPlane2_u8, const uint8_t * pPlane3_u8,
	const uint8_t *pPlane4_u8, const uint8_t *pPlane5_u8, const uint8_t *pPlane6_u8, const uint8_t * pPlane7_u8,
	const uint8_t *pPlane8_u8, const uint8_t *pPlane9_u8, const uint8_t *pPlane10_u8, const uint8_t * pPlane11_u8,
	const uint8_t *pPlane12_u8, const uint8_t * pPlane13_u8, const uint8_t *pPlane14_u8, const uint8_t * pPlane15_u8,
	int32_t width, int32_t height, int32_t pitch) {
	for (int32_t h = 0; h<height; h++) {
		for (int32_t w = 0; w<width; w++) {
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
		for (int32_t w = 0; w<width; w++) {
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
		for (int32_t w = 0; w<width; w++) {
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
		for (int32_t w = 0; w<width; w++) {
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

void Merge16PlanesToBig(uint8_t *pel4Plane, int32_t pel4Pitch,
	const uint8_t *pPlane0, const uint8_t *pPlane1, const uint8_t *pPlane2, const uint8_t * pPlane3,
	const uint8_t *pPlane4, const uint8_t *pPlane5, const uint8_t *pPlane6, const uint8_t * pPlane7,
	const uint8_t *pPlane8, const uint8_t *pPlane9, const uint8_t *pPlane10, const uint8_t * pPlane11,
	const uint8_t *pPlane12, const uint8_t * pPlane13, const uint8_t *pPlane14, const uint8_t * pPlane15,
	int32_t width, int32_t height, int32_t pitch) {
	RealMerge16PlanesToBig<float>(pel4Plane, pel4Pitch, pPlane0, pPlane1, pPlane2, pPlane3, pPlane4, pPlane5, pPlane6, pPlane7, pPlane8, pPlane9, pPlane10, pPlane11, pPlane12, pPlane13, pPlane14, pPlane15, width, height, pitch);
}

void MakeVectorSmallMasks(MVClipBalls *mvClip, int32_t nBlkX, int32_t nBlkY, uint8_t *VXSmallY, int32_t pitchVXSmallY, uint8_t*VYSmallY, int32_t pitchVYSmallY) {
	for (int32_t by = 0; by<nBlkY; by++)
		for (int32_t bx = 0; bx<nBlkX; bx++) {
			int32_t i = bx + by*nBlkX;
			const FakeBlockData &block = mvClip->GetBlock(0, i);
			int32_t vx = block.GetMV().x;
			int32_t vy = block.GetMV().y;
			if (vx>127) vx = 127;
			else if (vx<-127) vx = -127;
			if (vy>127) vy = 127;
			else if (vy<-127) vy = -127;
			VXSmallY[bx + by*pitchVXSmallY] = vx + 128;
			VYSmallY[bx + by*pitchVYSmallY] = vy + 128;
		}
}

void VectorSmallMaskYToHalfUV(uint8_t * VSmallY, int32_t nBlkX, int32_t nBlkY, uint8_t *VSmallUV, int32_t ratioUV) {
	if (ratioUV == 2) {
		for (int32_t by = 0; by<nBlkY; by++) {
			for (int32_t bx = 0; bx<nBlkX; bx++)
				VSmallUV[bx] = ((VSmallY[bx] - 128) >> 1) + 128;
			VSmallY += nBlkX;
			VSmallUV += nBlkX;
		}
	}
	else {
		for (int32_t by = 0; by<nBlkY; by++) {
			for (int32_t bx = 0; bx<nBlkX; bx++)
				VSmallUV[bx] = VSmallY[bx];
			VSmallY += nBlkX;
			VSmallUV += nBlkX;
		}
	}
}

template <typename PixelType>
void RealBlend(uint8_t * pdst, const uint8_t * psrc, const uint8_t * pref, int32_t height, int32_t width, int32_t dst_pitch, int32_t src_pitch, int32_t ref_pitch, int32_t time256) {
	int32_t h, w;
	for (h = 0; h<height; h++) {
		for (w = 0; w<width; w++) {
			const PixelType *psrc_ = (const PixelType *)psrc;
			const PixelType *pref_ = (const PixelType *)pref;
			PixelType *pdst_ = (PixelType *)pdst;
			pdst_[w] = (psrc_[w] * (256 - time256) + pref_[w] * time256) / 256;
		}
		pdst += dst_pitch;
		psrc += src_pitch;
		pref += ref_pitch;
	}
}

void Blend(uint8_t * pdst, const uint8_t * psrc, const uint8_t * pref, int32_t height, int32_t width, int32_t dst_pitch, int32_t src_pitch, int32_t ref_pitch, int32_t time256) {
	RealBlend<float>(pdst, psrc, pref, height, width, dst_pitch, src_pitch, ref_pitch, time256);
}

inline void ByteOccMask(uint8_t *occMask, int32_t occlusion, double occnorm, double fGamma) {
	if (fGamma == 1.0)
		*occMask = max(*occMask, min((int32_t)(255 * occlusion*occnorm), 255));
	else
		*occMask = max(*occMask, min((int32_t)(255 * pow(occlusion*occnorm, fGamma)), 255));
}

void MakeVectorOcclusionMaskTime(MVClipBalls *mvClip, int32_t nBlkX, int32_t nBlkY, double dMaskNormFactor, double fGamma, int32_t nPel, uint8_t * occMask, int32_t occMaskPitch, int32_t time256, int32_t blkSizeX, int32_t blkSizeY) {
	memset(occMask, 0, nBlkX * nBlkY);
	int32_t time4096X = time256 * 16 / blkSizeX;
	int32_t time4096Y = time256 * 16 / blkSizeY;
	double occnorm = 10 / dMaskNormFactor / nPel;
	int32_t occlusion;
	for (int32_t by = 0; by<nBlkY; by++)
		for (int32_t bx = 0; bx<nBlkX; bx++) {
			int32_t i = bx + by*nBlkX;
			const FakeBlockData &block = mvClip->GetBlock(0, i);
			int32_t vx = block.GetMV().x;
			int32_t vy = block.GetMV().y;
			if (bx < nBlkX - 1) {
				int32_t i1 = i + 1;
				const FakeBlockData &block1 = mvClip->GetBlock(0, i1);
				int32_t vx1 = block1.GetMV().x;
				if (vx1<vx) {
					occlusion = vx - vx1;
					for (int32_t bxi = bx + vx1*time4096X / 4096; bxi <= bx + vx*time4096X / 4096 + 1 && bxi >= 0 && bxi<nBlkX; bxi++)
						ByteOccMask(&occMask[bxi + by*occMaskPitch], occlusion, occnorm, fGamma);
				}
			}
			if (by < nBlkY - 1) {
				int32_t i1 = i + nBlkX;
				const FakeBlockData &block1 = mvClip->GetBlock(0, i1);
				int32_t vy1 = block1.GetMV().y;
				if (vy1<vy) {
					occlusion = vy - vy1;
					for (int32_t byi = by + vy1*time4096Y / 4096; byi <= by + vy*time4096Y / 4096 + 1 && byi >= 0 && byi<nBlkY; byi++)
						ByteOccMask(&occMask[bx + byi*occMaskPitch], occlusion, occnorm, fGamma);
				}
			}
		}
}

inline float Median3r(float a, float b, float c) {
	if (b <= a) return a;
	else  if (c <= b) return c;
	else return b;
}

template <typename PixelType>
void RealFlowInterExtra(uint8_t * pdst8, int32_t dst_pitch, const uint8_t *prefB8, const uint8_t *prefF8, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, const int32_t *LUTVB, const int32_t * LUTVF,
	uint8_t *VXFullBB, uint8_t *VXFullFF, uint8_t *VYFullBB, uint8_t *VYFullFF) {
	const PixelType *prefB = (const PixelType *)prefB8;
	const PixelType *prefF = (const PixelType *)prefF8;
	PixelType *pdst = (PixelType *)pdst8;
	ref_pitch /= sizeof(PixelType);
	dst_pitch /= sizeof(PixelType);
	if (nPel == 1) {
		for (int32_t h = 0; h<height; h++) {
			for (int32_t w = 0; w<width; w++) {
				int32_t vxF = LUTVF[VXFullF[w]];
				int32_t vyF = LUTVF[VYFullF[w]];
				int32_t adrF = vyF*ref_pitch + vxF + w;
				float dstF = prefF[adrF];
				int32_t vxFF = LUTVF[VXFullFF[w]];
				int32_t vyFF = LUTVF[VYFullFF[w]];
				int32_t adrFF = vyFF*ref_pitch + vxFF + w;
				float dstFF = prefF[adrFF];
				int32_t vxB = LUTVB[VXFullB[w]];
				int32_t vyB = LUTVB[VYFullB[w]];
				int32_t adrB = vyB*ref_pitch + vxB + w;
				float dstB = prefB[adrB];
				int32_t vxBB = LUTVB[VXFullBB[w]];
				int32_t vyBB = LUTVB[VYFullBB[w]];
				int32_t adrBB = vyBB*ref_pitch + vxBB + w;
				float dstBB = prefB[adrBB];
				float minfb;
				float maxfb;
				if (dstF > dstB) {
					minfb = dstB;
					maxfb = dstF;
				}
				else {
					maxfb = dstB;
					minfb = dstF;
				}
				pdst[w] = (((Median3r(minfb, dstBB, maxfb)*MaskF[w] + dstF*(255 - MaskF[w])) / 256)*(256 - time256) +
					((Median3r(minfb, dstFF, maxfb)*MaskB[w] + dstB*(255 - MaskB[w])) / 256)*time256) / 256;
			}
			pdst += dst_pitch;
			prefB += ref_pitch;
			prefF += ref_pitch;
			VXFullB += VPitch;
			VYFullB += VPitch;
			VXFullF += VPitch;
			VYFullF += VPitch;
			MaskB += VPitch;
			MaskF += VPitch;
			VXFullBB += VPitch;
			VYFullBB += VPitch;
			VXFullFF += VPitch;
			VYFullFF += VPitch;
		}
	}
	else if (nPel == 2) {
		for (int32_t h = 0; h<height; h++) {
			for (int32_t w = 0; w<width; w++) {
				int32_t vxF = LUTVF[VXFullF[w]];
				int32_t vyF = LUTVF[VYFullF[w]];
				int32_t adrF = vyF*ref_pitch + vxF + (w << 1);
				float dstF = prefF[adrF];
				int32_t vxFF = LUTVF[VXFullFF[w]];
				int32_t vyFF = LUTVF[VYFullFF[w]];
				int32_t adrFF = vyFF*ref_pitch + vxFF + (w << 1);
				float dstFF = prefF[adrFF];
				int32_t vxB = LUTVB[VXFullB[w]];
				int32_t vyB = LUTVB[VYFullB[w]];
				int32_t adrB = vyB*ref_pitch + vxB + (w << 1);
				float dstB = prefB[adrB];
				int32_t vxBB = LUTVB[VXFullBB[w]];
				int32_t vyBB = LUTVB[VYFullBB[w]];
				int32_t adrBB = vyBB*ref_pitch + vxBB + (w << 1);
				float dstBB = prefB[adrBB];
				float minfb;
				float maxfb;
				if (dstF > dstB) {
					minfb = dstB;
					maxfb = dstF;
				}
				else {
					maxfb = dstB;
					minfb = dstF;
				}
				pdst[w] = (((Median3r(minfb, dstBB, maxfb)*MaskF[w] + dstF*(255 - MaskF[w])) / 256)*(256 - time256) +
					((Median3r(minfb, dstFF, maxfb)*MaskB[w] + dstB*(255 - MaskB[w])) / 256)*time256) / 256;
			}
			pdst += dst_pitch;
			prefB += ref_pitch << 1;
			prefF += ref_pitch << 1;
			VXFullB += VPitch;
			VYFullB += VPitch;
			VXFullF += VPitch;
			VYFullF += VPitch;
			MaskB += VPitch;
			MaskF += VPitch;
			VXFullBB += VPitch;
			VYFullBB += VPitch;
			VXFullFF += VPitch;
			VYFullFF += VPitch;
		}
	}
	else if (nPel == 4) {
		for (int32_t h = 0; h<height; h++) {
			for (int32_t w = 0; w<width; w++) {
				int32_t vxF = LUTVF[VXFullF[w]];
				int32_t vyF = LUTVF[VYFullF[w]];
				float dstF = prefF[vyF*ref_pitch + vxF + (w << 2)];
				int32_t vxFF = LUTVF[VXFullFF[w]];
				int32_t vyFF = LUTVF[VYFullFF[w]];
				float dstFF = prefF[vyFF*ref_pitch + vxFF + (w << 2)];
				int32_t vxB = LUTVB[VXFullB[w]];
				int32_t vyB = LUTVB[VYFullB[w]];
				float dstB = prefB[vyB*ref_pitch + vxB + (w << 2)];
				int32_t vxBB = LUTVB[VXFullBB[w]];
				int32_t vyBB = LUTVB[VYFullBB[w]];
				float dstBB = prefB[vyBB*ref_pitch + vxBB + (w << 2)];
				float minfb;
				float maxfb;
				if (dstF > dstB) {
					minfb = dstB;
					maxfb = dstF;
				}
				else {
					maxfb = dstB;
					minfb = dstF;
				}
				pdst[w] = (((Median3r(minfb, dstBB, maxfb)*MaskF[w] + dstF*(255 - MaskF[w])) / 256)*(256 - time256) +
					((Median3r(minfb, dstFF, maxfb)*MaskB[w] + dstB*(255 - MaskB[w])) / 256)*time256) / 256;
			}
			pdst += dst_pitch;
			prefB += ref_pitch << 2;
			prefF += ref_pitch << 2;
			VXFullB += VPitch;
			VYFullB += VPitch;
			VXFullF += VPitch;
			VYFullF += VPitch;
			MaskB += VPitch;
			MaskF += VPitch;
			VXFullBB += VPitch;
			VYFullBB += VPitch;
			VXFullFF += VPitch;
			VYFullFF += VPitch;
		}
	}
}

void FlowInterExtra(uint8_t * pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, const int32_t *LUTVB, const int32_t * LUTVF,
	uint8_t *VXFullBB, uint8_t *VXFullFF, uint8_t *VYFullBB, uint8_t *VYFullFF) {
	RealFlowInterExtra<float>(pdst, dst_pitch, prefB, prefF, ref_pitch, VXFullB, VXFullF, VYFullB, VYFullF, MaskB, MaskF, VPitch, width, height, time256, nPel, LUTVB, LUTVF, VXFullBB, VXFullFF, VYFullBB, VYFullFF);
}

template <typename PixelType>
void RealFlowInter(uint8_t * pdst8, int32_t dst_pitch, const uint8_t *prefB8, const uint8_t *prefF8, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, const int32_t *LUTVB, const int32_t *LUTVF) {
	const PixelType *prefB = (const PixelType *)prefB8;
	const PixelType *prefF = (const PixelType *)prefF8;
	PixelType *pdst = (PixelType *)pdst8;
	ref_pitch /= sizeof(PixelType);
	dst_pitch /= sizeof(PixelType);
	if (nPel == 1) {
		for (int32_t h = 0; h<height; h++) {
			for (int32_t w = 0; w<width; w++) {
				int32_t vxF = LUTVF[VXFullF[w]];
				int32_t vyF = LUTVF[VYFullF[w]];
				double dstF = prefF[vyF*ref_pitch + vxF + w];
				float dstF0 = prefF[w];
				int32_t vxB = LUTVB[VXFullB[w]];
				int32_t vyB = LUTVB[VYFullB[w]];
				double dstB = prefB[vyB*ref_pitch + vxB + w];
				float dstB0 = prefB[w];
				pdst[w] = PixelType((((dstF*(255 - MaskF[w]) + ((MaskF[w] * (dstB*(255 - MaskB[w]) + MaskB[w] * dstF0)) / 256)) / 256)*(256 - time256) +
					((dstB*(255 - MaskB[w]) + ((MaskB[w] * (dstF*(255 - MaskF[w]) + MaskF[w] * dstB0)) / 256)) / 256)*time256) / 256);
			}
			pdst += dst_pitch;
			prefB += ref_pitch;
			prefF += ref_pitch;
			VXFullB += VPitch;
			VYFullB += VPitch;
			VXFullF += VPitch;
			VYFullF += VPitch;
			MaskB += VPitch;
			MaskF += VPitch;
		}
	}
	else if (nPel == 2) {
		for (int32_t h = 0; h<height; h++) {
			for (int32_t w = 0; w<width; w++) {
				int32_t vxF = LUTVF[VXFullF[w]];
				int32_t vyF = LUTVF[VYFullF[w]];
				float dstF = prefF[vyF*ref_pitch + vxF + (w << 1)];
				float dstF0 = prefF[(w << 1)];
				int32_t vxB = LUTVB[VXFullB[w]];
				int32_t vyB = LUTVB[VYFullB[w]];
				float dstB = prefB[vyB*ref_pitch + vxB + (w << 1)];
				float dstB0 = prefB[(w << 1)];
				pdst[w] = (((dstF*(255 - MaskF[w]) + ((MaskF[w] * (dstB*(255 - MaskB[w]) + MaskB[w] * dstF0)) / 256)) / 256)*(256 - time256) +
					((dstB*(255 - MaskB[w]) + ((MaskB[w] * (dstF*(255 - MaskF[w]) + MaskF[w] * dstB0)) / 256)) / 256)*time256) / 256;
			}
			pdst += dst_pitch;
			prefB += ref_pitch << 1;
			prefF += ref_pitch << 1;
			VXFullB += VPitch;
			VYFullB += VPitch;
			VXFullF += VPitch;
			VYFullF += VPitch;
			MaskB += VPitch;
			MaskF += VPitch;
		}
	}
	else if (nPel == 4) {
		for (int32_t h = 0; h<height; h++) {
			for (int32_t w = 0; w<width; w++) {
				int32_t vxF = LUTVF[VXFullF[w]];
				int32_t vyF = LUTVF[VYFullF[w]];
				float dstF = prefF[vyF*ref_pitch + vxF + (w << 2)];
				float dstF0 = prefF[(w << 2)];
				int32_t vxB = LUTVB[VXFullB[w]];
				int32_t vyB = LUTVB[VYFullB[w]];
				float dstB = prefB[vyB*ref_pitch + vxB + (w << 2)];
				float dstB0 = prefB[(w << 2)];
				pdst[w] = (((dstF*(255 - MaskF[w]) + ((MaskF[w] * (dstB*(255 - MaskB[w]) + MaskB[w] * dstF0)) / 256)) / 256)*(256 - time256) +
					((dstB*(255 - MaskB[w]) + ((MaskB[w] * (dstF*(255 - MaskF[w]) + MaskF[w] * dstB0)) / 256)) / 256)*time256) / 256;
			}
			pdst += dst_pitch;
			prefB += ref_pitch << 2;
			prefF += ref_pitch << 2;
			VXFullB += VPitch;
			VYFullB += VPitch;
			VXFullF += VPitch;
			VYFullF += VPitch;
			MaskB += VPitch;
			MaskF += VPitch;
		}
	}
}

void FlowInter(uint8_t * pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, const int32_t *LUTVB, const int32_t *LUTVF) {
	RealFlowInter<float>(pdst, dst_pitch, prefB, prefF, ref_pitch, VXFullB, VXFullF, VYFullB, VYFullF, MaskB, MaskF, VPitch, width, height, time256, nPel, LUTVB, LUTVF);
}

void Create_LUTV(int32_t time256, int32_t *LUTVB, int32_t *LUTVF) {
	for (int32_t v = 0; v<256; v++) {
		LUTVB[v] = ((v - 128)*(256 - time256)) / 256;
		LUTVF[v] = ((v - 128)*time256) / 256;
	}
}

template <typename PixelType>
void RealFlowInterSimple(uint8_t * pdst8, int32_t dst_pitch, const uint8_t *prefB8, const uint8_t *prefF8, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, int32_t *LUTVB, int32_t *LUTVF) {
	const PixelType *prefB = (const PixelType *)prefB8;
	const PixelType *prefF = (const PixelType *)prefF8;
	PixelType *pdst = (PixelType *)pdst8;
	ref_pitch /= sizeof(PixelType);
	dst_pitch /= sizeof(PixelType);
	if (time256 == 128) {
		if (nPel == 1) {
			for (int32_t h = 0; h<height; h++) {
				for (int32_t w = 0; w<width; w += 2) {
					int32_t vxF = (VXFullF[w] - 128) >> 1;
					int32_t vyF = (VYFullF[w] - 128) >> 1;
					int32_t addrF = vyF*ref_pitch + vxF + w;
					float dstF = prefF[addrF];
					float dstF1 = prefF[addrF + 1];
					int32_t vxB = (VXFullB[w] - 128) >> 1;
					int32_t vyB = (VYFullB[w] - 128) >> 1;
					int32_t addrB = vyB*ref_pitch + vxB + w;
					float dstB = prefB[addrB];
					float dstB1 = prefB[addrB + 1];
					pdst[w] = (((dstF + dstB) * 256) + (dstB - dstF)*(MaskF[w] - MaskB[w])) / 512;
					pdst[w + 1] = (((dstF1 + dstB1) * 256) + (dstB1 - dstF1)*(MaskF[w + 1] - MaskB[w + 1])) / 512;
				}
				pdst += dst_pitch;
				prefB += ref_pitch;
				prefF += ref_pitch;
				VXFullB += VPitch;
				VYFullB += VPitch;
				VXFullF += VPitch;
				VYFullF += VPitch;
				MaskB += VPitch;
				MaskF += VPitch;
			}
		}
		else if (nPel == 2) {
			for (int32_t h = 0; h<height; h++) {
				for (int32_t w = 0; w<width; w += 1) {
					int32_t vxF = (VXFullF[w] - 128) >> 1;
					int32_t vyF = (VYFullF[w] - 128) >> 1;
					float dstF = prefF[vyF*ref_pitch + vxF + (w << 1)];
					int32_t vxB = (VXFullB[w] - 128) >> 1;
					int32_t vyB = (VYFullB[w] - 128) >> 1;
					float dstB = prefB[vyB*ref_pitch + vxB + (w << 1)];
					pdst[w] = (((dstF + dstB) * 256) + (dstB - dstF)*(MaskF[w] - MaskB[w])) / 512;
				}
				pdst += dst_pitch;
				prefB += ref_pitch << 1;
				prefF += ref_pitch << 1;
				VXFullB += VPitch;
				VYFullB += VPitch;
				VXFullF += VPitch;
				VYFullF += VPitch;
				MaskB += VPitch;
				MaskF += VPitch;
			}
		}
		else if (nPel == 4) {
			for (int32_t h = 0; h<height; h++) {
				for (int32_t w = 0; w<width; w += 1) {
					int32_t vxF = (VXFullF[w] - 128) >> 1;
					int32_t vyF = (VYFullF[w] - 128) >> 1;
					float dstF = prefF[vyF*ref_pitch + vxF + (w << 2)];
					int32_t vxB = (VXFullB[w] - 128) >> 1;
					int32_t vyB = (VYFullB[w] - 128) >> 1;
					float dstB = prefB[vyB*ref_pitch + vxB + (w << 2)];
					pdst[w] = (((dstF + dstB) * 256) + (dstB - dstF)*(MaskF[w] - MaskB[w])) / 512;
				}
				pdst += dst_pitch;
				prefB += ref_pitch << 2;
				prefF += ref_pitch << 2;
				VXFullB += VPitch;
				VYFullB += VPitch;
				VXFullF += VPitch;
				VYFullF += VPitch;
				MaskB += VPitch;
				MaskF += VPitch;
			}
		}
	}
	else {
		if (nPel == 1) {
			for (int32_t h = 0; h<height; h++) {
				for (int32_t w = 0; w<width; w += 2) {
					int32_t vxF = LUTVF[VXFullF[w]];
					int32_t vyF = LUTVF[VYFullF[w]];
					int32_t addrF = vyF*ref_pitch + vxF + w;
					float dstF = prefF[addrF];
					float dstF1 = prefF[addrF + 1];
					int32_t vxB = LUTVB[VXFullB[w]];
					int32_t vyB = LUTVB[VYFullB[w]];
					int32_t addrB = vyB*ref_pitch + vxB + w;
					float dstB = prefB[addrB];
					float dstB1 = prefB[addrB + 1];
					pdst[w] = (((dstF * 255 + (dstB - dstF)*MaskF[w]))*(256 - time256) +
						((dstB * 255 - (dstB - dstF)*MaskB[w]))*time256) / 65536;
					pdst[w + 1] = (((dstF1 * 255 + (dstB1 - dstF1)*MaskF[w + 1]))*(256 - time256) +
						((dstB1 * 255 - (dstB1 - dstF1)*MaskB[w + 1]))*time256) / 65536;
				}
				pdst += dst_pitch;
				prefB += ref_pitch;
				prefF += ref_pitch;
				VXFullB += VPitch;
				VYFullB += VPitch;
				VXFullF += VPitch;
				VYFullF += VPitch;
				MaskB += VPitch;
				MaskF += VPitch;
			}
		}
		else if (nPel == 2) {
			for (int32_t h = 0; h<height; h++) {
				for (int32_t w = 0; w<width; w += 1) {
					int32_t vxF = LUTVF[VXFullF[w]];
					int32_t vyF = LUTVF[VYFullF[w]];
					float dstF = prefF[vyF*ref_pitch + vxF + (w << 1)];
					int32_t vxB = LUTVB[VXFullB[w]];
					int32_t vyB = LUTVB[VYFullB[w]];
					float dstB = prefB[vyB*ref_pitch + vxB + (w << 1)];
					pdst[w] = (((dstF*(255 - MaskF[w]) + dstB*MaskF[w]) / 256)*(256 - time256) +
						((dstB*(255 - MaskB[w]) + dstF*MaskB[w]) / 256)*time256) / 256;
				}
				pdst += dst_pitch;
				prefB += ref_pitch << 1;
				prefF += ref_pitch << 1;
				VXFullB += VPitch;
				VYFullB += VPitch;
				VXFullF += VPitch;
				VYFullF += VPitch;
				MaskB += VPitch;
				MaskF += VPitch;
			}
		}
		else if (nPel == 4) {
			for (int32_t h = 0; h<height; h++) {
				for (int32_t w = 0; w<width; w += 1) {
					int32_t vxF = LUTVF[VXFullF[w]];
					int32_t vyF = LUTVF[VYFullF[w]];
					float dstF = prefF[vyF*ref_pitch + vxF + (w << 2)];
					int32_t vxB = LUTVB[VXFullB[w]];
					int32_t vyB = LUTVB[VYFullB[w]];
					float dstB = prefB[vyB*ref_pitch + vxB + (w << 2)];
					pdst[w] = (((dstF*(255 - MaskF[w]) + dstB*MaskF[w]) / 256)*(256 - time256) +
						((dstB*(255 - MaskB[w]) + dstF*MaskB[w]) / 256)*time256) / 256;
				}
				pdst += dst_pitch;
				prefB += ref_pitch << 2;
				prefF += ref_pitch << 2;
				VXFullB += VPitch;
				VYFullB += VPitch;
				VXFullF += VPitch;
				VYFullF += VPitch;
				MaskB += VPitch;
				MaskF += VPitch;
			}
		}
	}
}

void FlowInterSimple(uint8_t * pdst, int32_t dst_pitch, const uint8_t *prefB, const uint8_t *prefF, int32_t ref_pitch,
	uint8_t *VXFullB, uint8_t *VXFullF, uint8_t *VYFullB, uint8_t *VYFullF, uint8_t *MaskB, uint8_t *MaskF,
	int32_t VPitch, int32_t width, int32_t height, int32_t time256, int32_t nPel, int32_t *LUTVB, int32_t *LUTVF) {
	RealFlowInterSimple<float>(pdst, dst_pitch, prefB, prefF, ref_pitch, VXFullB, VXFullF, VYFullB, VYFullF, MaskB, MaskF, VPitch, width, height, time256, nPel, LUTVB, LUTVF);
}
