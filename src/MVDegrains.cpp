#include "VapourSynth.h"
#include "VSHelper.h"
#include "MVClip.h"
#include "MVDegrains.h"
#include "MVFilter.h"
#include "MVFrame.h"
#include "MVInterface.h"
#include "Overlap.h"

struct MVDegrainData {
	VSNodeRef *node;
	const VSVideoInfo *vi;
	VSNodeRef *super;
	VSNodeRef *vectors[48];
	double thSAD[3];
	int32_t YUVplanes;
	double nLimit[3];
	double nSCD1;
	double nSCD2;
	MVClipDicks *mvClips[48];
	MVFilter *bleh;
	int32_t nSuperHPad;
	int32_t nSuperVPad;
	int32_t nSuperPel;
	int32_t nSuperModeYUV;
	int32_t nSuperLevels;
	int32_t dstTempPitch;
	OverlapsFunction OVERS[3];
	DenoiseFunction DEGRAIN[3];
	LimitFunction LimitChanges;
	ToPixelsFunction ToPixels;
	bool process[3];
	int32_t xSubUV;
	int32_t ySubUV;
	int32_t nWidth[3];
	int32_t nHeight[3];
	int32_t nOverlapX[3];
	int32_t nOverlapY[3];
	int32_t nBlkSizeX[3];
	int32_t nBlkSizeY[3];
	int32_t nWidth_B[3];
	int32_t nHeight_B[3];
	OverlapWindows *OverWins[3];
};

static void VS_CC mvdegrainInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVDegrainData *d = reinterpret_cast<MVDegrainData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

template <int32_t radius>
static const VSFrameRef *VS_CC mvdegrainGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVDegrainData *d = reinterpret_cast<MVDegrainData *>(*instanceData);
	if (activationReason == arInitial) {
		if (radius > 23)
			vsapi->requestFrameFilter(n, d->vectors[Forward24], frameCtx);
		if (radius > 22)
			vsapi->requestFrameFilter(n, d->vectors[Forward23], frameCtx);
		if (radius > 21)
			vsapi->requestFrameFilter(n, d->vectors[Forward22], frameCtx);
		if (radius > 20)
			vsapi->requestFrameFilter(n, d->vectors[Forward21], frameCtx);
		if (radius > 19)
			vsapi->requestFrameFilter(n, d->vectors[Forward20], frameCtx);
		if (radius > 18)
			vsapi->requestFrameFilter(n, d->vectors[Forward19], frameCtx);
		if (radius > 17)
			vsapi->requestFrameFilter(n, d->vectors[Forward18], frameCtx);
		if (radius > 16)
			vsapi->requestFrameFilter(n, d->vectors[Forward17], frameCtx);
		if (radius > 15)
			vsapi->requestFrameFilter(n, d->vectors[Forward16], frameCtx);
		if (radius > 14)
			vsapi->requestFrameFilter(n, d->vectors[Forward15], frameCtx);
		if (radius > 13)
			vsapi->requestFrameFilter(n, d->vectors[Forward14], frameCtx);
		if (radius > 12)
			vsapi->requestFrameFilter(n, d->vectors[Forward13], frameCtx);
		if (radius > 11)
			vsapi->requestFrameFilter(n, d->vectors[Forward12], frameCtx);
		if (radius > 10)
			vsapi->requestFrameFilter(n, d->vectors[Forward11], frameCtx);
		if (radius > 9)
			vsapi->requestFrameFilter(n, d->vectors[Forward10], frameCtx);
		if (radius > 8)
			vsapi->requestFrameFilter(n, d->vectors[Forward9], frameCtx);
		if (radius > 7)
			vsapi->requestFrameFilter(n, d->vectors[Forward8], frameCtx);
		if (radius > 6)
			vsapi->requestFrameFilter(n, d->vectors[Forward7], frameCtx);
		if (radius > 5)
			vsapi->requestFrameFilter(n, d->vectors[Forward6], frameCtx);
		if (radius > 4)
			vsapi->requestFrameFilter(n, d->vectors[Forward5], frameCtx);
		if (radius > 3)
			vsapi->requestFrameFilter(n, d->vectors[Forward4], frameCtx);
		if (radius > 2)
			vsapi->requestFrameFilter(n, d->vectors[Forward3], frameCtx);
		if (radius > 1)
			vsapi->requestFrameFilter(n, d->vectors[Forward2], frameCtx);
		vsapi->requestFrameFilter(n, d->vectors[Forward1], frameCtx);
		vsapi->requestFrameFilter(n, d->vectors[Backward1], frameCtx);
		if (radius > 1)
			vsapi->requestFrameFilter(n, d->vectors[Backward2], frameCtx);
		if (radius > 2)
			vsapi->requestFrameFilter(n, d->vectors[Backward3], frameCtx);
		if (radius > 3)
			vsapi->requestFrameFilter(n, d->vectors[Backward4], frameCtx);
		if (radius > 4)
			vsapi->requestFrameFilter(n, d->vectors[Backward5], frameCtx);
		if (radius > 5)
			vsapi->requestFrameFilter(n, d->vectors[Backward6], frameCtx);
		if (radius > 6)
			vsapi->requestFrameFilter(n, d->vectors[Backward7], frameCtx);
		if (radius > 7)
			vsapi->requestFrameFilter(n, d->vectors[Backward8], frameCtx);
		if (radius > 8)
			vsapi->requestFrameFilter(n, d->vectors[Backward9], frameCtx);
		if (radius > 9)
			vsapi->requestFrameFilter(n, d->vectors[Backward10], frameCtx);
		if (radius > 10)
			vsapi->requestFrameFilter(n, d->vectors[Backward11], frameCtx);
		if (radius > 11)
			vsapi->requestFrameFilter(n, d->vectors[Backward12], frameCtx);
		if (radius > 12)
			vsapi->requestFrameFilter(n, d->vectors[Backward13], frameCtx);
		if (radius > 13)
			vsapi->requestFrameFilter(n, d->vectors[Backward14], frameCtx);
		if (radius > 14)
			vsapi->requestFrameFilter(n, d->vectors[Backward15], frameCtx);
		if (radius > 15)
			vsapi->requestFrameFilter(n, d->vectors[Backward16], frameCtx);
		if (radius > 16)
			vsapi->requestFrameFilter(n, d->vectors[Backward17], frameCtx);
		if (radius > 17)
			vsapi->requestFrameFilter(n, d->vectors[Backward18], frameCtx);
		if (radius > 18)
			vsapi->requestFrameFilter(n, d->vectors[Backward19], frameCtx);
		if (radius > 19)
			vsapi->requestFrameFilter(n, d->vectors[Backward20], frameCtx);
		if (radius > 20)
			vsapi->requestFrameFilter(n, d->vectors[Backward21], frameCtx);
		if (radius > 21)
			vsapi->requestFrameFilter(n, d->vectors[Backward22], frameCtx);
		if (radius > 22)
			vsapi->requestFrameFilter(n, d->vectors[Backward23], frameCtx);
		if (radius > 23)
			vsapi->requestFrameFilter(n, d->vectors[Backward24], frameCtx);
		if (radius > 23) {
			int32_t offF24 = -1 * d->mvClips[Forward24]->GetDeltaFrame();
			if (n + offF24 >= 0)
				vsapi->requestFrameFilter(n + offF24, d->super, frameCtx);
		}
		if (radius > 22) {
			int32_t offF23 = -1 * d->mvClips[Forward23]->GetDeltaFrame();
			if (n + offF23 >= 0)
				vsapi->requestFrameFilter(n + offF23, d->super, frameCtx);
		}
		if (radius > 21) {
			int32_t offF22 = -1 * d->mvClips[Forward22]->GetDeltaFrame();
			if (n + offF22 >= 0)
				vsapi->requestFrameFilter(n + offF22, d->super, frameCtx);
		}
		if (radius > 20) {
			int32_t offF21 = -1 * d->mvClips[Forward21]->GetDeltaFrame();
			if (n + offF21 >= 0)
				vsapi->requestFrameFilter(n + offF21, d->super, frameCtx);
		}
		if (radius > 19) {
			int32_t offF20 = -1 * d->mvClips[Forward20]->GetDeltaFrame();
			if (n + offF20 >= 0)
				vsapi->requestFrameFilter(n + offF20, d->super, frameCtx);
		}
		if (radius > 18) {
			int32_t offF19 = -1 * d->mvClips[Forward19]->GetDeltaFrame();
			if (n + offF19 >= 0)
				vsapi->requestFrameFilter(n + offF19, d->super, frameCtx);
		}
		if (radius > 17) {
			int32_t offF18 = -1 * d->mvClips[Forward18]->GetDeltaFrame();
			if (n + offF18 >= 0)
				vsapi->requestFrameFilter(n + offF18, d->super, frameCtx);
		}
		if (radius > 16) {
			int32_t offF17 = -1 * d->mvClips[Forward17]->GetDeltaFrame();
			if (n + offF17 >= 0)
				vsapi->requestFrameFilter(n + offF17, d->super, frameCtx);
		}
		if (radius > 15) {
			int32_t offF16 = -1 * d->mvClips[Forward16]->GetDeltaFrame();
			if (n + offF16 >= 0)
				vsapi->requestFrameFilter(n + offF16, d->super, frameCtx);
		}
		if (radius > 14) {
			int32_t offF15 = -1 * d->mvClips[Forward15]->GetDeltaFrame();
			if (n + offF15 >= 0)
				vsapi->requestFrameFilter(n + offF15, d->super, frameCtx);
		}
		if (radius > 13) {
			int32_t offF14 = -1 * d->mvClips[Forward14]->GetDeltaFrame();
			if (n + offF14 >= 0)
				vsapi->requestFrameFilter(n + offF14, d->super, frameCtx);
		}
		if (radius > 12) {
			int32_t offF13 = -1 * d->mvClips[Forward13]->GetDeltaFrame();
			if (n + offF13 >= 0)
				vsapi->requestFrameFilter(n + offF13, d->super, frameCtx);
		}
		if (radius > 11) {
			int32_t offF12 = -1 * d->mvClips[Forward12]->GetDeltaFrame();
			if (n + offF12 >= 0)
				vsapi->requestFrameFilter(n + offF12, d->super, frameCtx);
		}
		if (radius > 10) {
			int32_t offF11 = -1 * d->mvClips[Forward11]->GetDeltaFrame();
			if (n + offF11 >= 0)
				vsapi->requestFrameFilter(n + offF11, d->super, frameCtx);
		}
		if (radius > 9) {
			int32_t offF10 = -1 * d->mvClips[Forward10]->GetDeltaFrame();
			if (n + offF10 >= 0)
				vsapi->requestFrameFilter(n + offF10, d->super, frameCtx);
		}
		if (radius > 8) {
			int32_t offF9 = -1 * d->mvClips[Forward9]->GetDeltaFrame();
			if (n + offF9 >= 0)
				vsapi->requestFrameFilter(n + offF9, d->super, frameCtx);
		}
		if (radius > 7) {
			int32_t offF8 = -1 * d->mvClips[Forward8]->GetDeltaFrame();
			if (n + offF8 >= 0)
				vsapi->requestFrameFilter(n + offF8, d->super, frameCtx);
		}
		if (radius > 6) {
			int32_t offF7 = -1 * d->mvClips[Forward7]->GetDeltaFrame();
			if (n + offF7 >= 0)
				vsapi->requestFrameFilter(n + offF7, d->super, frameCtx);
		}
		if (radius > 5) {
			int32_t offF6 = -1 * d->mvClips[Forward6]->GetDeltaFrame();
			if (n + offF6 >= 0)
				vsapi->requestFrameFilter(n + offF6, d->super, frameCtx);
		}
		if (radius > 4) {
			int32_t offF5 = -1 * d->mvClips[Forward5]->GetDeltaFrame();
			if (n + offF5 >= 0)
				vsapi->requestFrameFilter(n + offF5, d->super, frameCtx);
		}
		if (radius > 3) {
			int32_t offF4 = -1 * d->mvClips[Forward4]->GetDeltaFrame();
			if (n + offF4 >= 0)
				vsapi->requestFrameFilter(n + offF4, d->super, frameCtx);
		}
		if (radius > 2) {
			int32_t offF3 = -1 * d->mvClips[Forward3]->GetDeltaFrame();
			if (n + offF3 >= 0)
				vsapi->requestFrameFilter(n + offF3, d->super, frameCtx);
		}
		if (radius > 1) {
			int32_t offF2 = -1 * d->mvClips[Forward2]->GetDeltaFrame();
			if (n + offF2 >= 0)
				vsapi->requestFrameFilter(n + offF2, d->super, frameCtx);
		}
		int32_t offF = -1 * d->mvClips[Forward1]->GetDeltaFrame();
		if (n + offF >= 0)
			vsapi->requestFrameFilter(n + offF, d->super, frameCtx);
		int32_t offB = d->mvClips[Backward1]->GetDeltaFrame();
		if (n + offB < d->vi->numFrames || !d->vi->numFrames)
			vsapi->requestFrameFilter(n + offB, d->super, frameCtx);
		if (radius > 1) {
			int32_t offB2 = d->mvClips[Backward2]->GetDeltaFrame();
			if (n + offB2 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB2, d->super, frameCtx);
		}
		if (radius > 2) {
			int32_t offB3 = d->mvClips[Backward3]->GetDeltaFrame();
			if (n + offB3 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB3, d->super, frameCtx);
		}
		if (radius > 3) {
			int32_t offB4 = d->mvClips[Backward4]->GetDeltaFrame();
			if (n + offB4 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB4, d->super, frameCtx);
		}
		if (radius > 4) {
			int32_t offB5 = d->mvClips[Backward5]->GetDeltaFrame();
			if (n + offB5 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB5, d->super, frameCtx);
		}
		if (radius > 5) {
			int32_t offB6 = d->mvClips[Backward6]->GetDeltaFrame();
			if (n + offB6 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB6, d->super, frameCtx);
		}
		if (radius > 6) {
			int32_t offB7 = d->mvClips[Backward7]->GetDeltaFrame();
			if (n + offB7 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB7, d->super, frameCtx);
		}
		if (radius > 7) {
			int32_t offB8 = d->mvClips[Backward8]->GetDeltaFrame();
			if (n + offB8 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB8, d->super, frameCtx);
		}
		if (radius > 8) {
			int32_t offB9 = d->mvClips[Backward9]->GetDeltaFrame();
			if (n + offB9 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB9, d->super, frameCtx);
		}
		if (radius > 9) {
			int32_t offB10 = d->mvClips[Backward10]->GetDeltaFrame();
			if (n + offB10 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB10, d->super, frameCtx);
		}
		if (radius > 10) {
			int32_t offB11 = d->mvClips[Backward11]->GetDeltaFrame();
			if (n + offB11 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB11, d->super, frameCtx);
		}
		if (radius > 11) {
			int32_t offB12 = d->mvClips[Backward12]->GetDeltaFrame();
			if (n + offB12 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB12, d->super, frameCtx);
		}
		if (radius > 12) {
			int32_t offB13 = d->mvClips[Backward13]->GetDeltaFrame();
			if (n + offB13 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB13, d->super, frameCtx);
		}
		if (radius > 13) {
			int32_t offB14 = d->mvClips[Backward14]->GetDeltaFrame();
			if (n + offB14 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB14, d->super, frameCtx);
		}
		if (radius > 14) {
			int32_t offB15 = d->mvClips[Backward15]->GetDeltaFrame();
			if (n + offB15 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB15, d->super, frameCtx);
		}
		if (radius > 15) {
			int32_t offB16 = d->mvClips[Backward16]->GetDeltaFrame();
			if (n + offB16 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB16, d->super, frameCtx);
		}
		if (radius > 16) {
			int32_t offB17 = d->mvClips[Backward17]->GetDeltaFrame();
			if (n + offB17 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB17, d->super, frameCtx);
		}
		if (radius > 17) {
			int32_t offB18 = d->mvClips[Backward18]->GetDeltaFrame();
			if (n + offB18 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB18, d->super, frameCtx);
		}
		if (radius > 18) {
			int32_t offB19 = d->mvClips[Backward19]->GetDeltaFrame();
			if (n + offB19 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB19, d->super, frameCtx);
		}
		if (radius > 19) {
			int32_t offB20 = d->mvClips[Backward20]->GetDeltaFrame();
			if (n + offB20 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB20, d->super, frameCtx);
		}
		if (radius > 20) {
			int32_t offB21 = d->mvClips[Backward21]->GetDeltaFrame();
			if (n + offB21 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB21, d->super, frameCtx);
		}
		if (radius > 21) {
			int32_t offB22 = d->mvClips[Backward22]->GetDeltaFrame();
			if (n + offB22 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB22, d->super, frameCtx);
		}
		if (radius > 22) {
			int32_t offB23 = d->mvClips[Backward23]->GetDeltaFrame();
			if (n + offB23 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB23, d->super, frameCtx);
		}
		if (radius > 23) {
			int32_t offB24 = d->mvClips[Backward24]->GetDeltaFrame();
			if (n + offB24 < d->vi->numFrames || !d->vi->numFrames)
				vsapi->requestFrameFilter(n + offB24, d->super, frameCtx);
		}
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		VSFrameRef *dst = vsapi->newVideoFrame(d->vi->format, d->vi->width, d->vi->height, src, core);
		uint8_t *pDst[3], *pDstCur[3];
		const uint8_t *pSrcCur[3];
		const uint8_t *pSrc[3];
		const uint8_t *pRefs[3][radius * 2];
		int32_t nDstPitches[3], nSrcPitches[3];
		int32_t nRefPitches[3][radius * 2];
		bool isUsable[radius * 2];
		int32_t nLogPel = (d->bleh->nPel == 4) ? 2 : (d->bleh->nPel == 2) ? 1 : 0;
		MVClipBalls *balls[radius * 2];
		const VSFrameRef *refFrames[radius * 2] = { 0 };
		for (int32_t r = 0; r < radius * 2; r++) {
			const VSFrameRef *frame = vsapi->getFrameFilter(n, d->vectors[r], frameCtx);
			balls[r] = new MVClipBalls(d->mvClips[r], vsapi);
			balls[r]->Update(frame);
			isUsable[r] = balls[r]->IsUsable();
			vsapi->freeFrame(frame);
			if (isUsable[r]) {
				int32_t offset = d->mvClips[r]->GetDeltaFrame() * (d->mvClips[r]->IsBackward() ? 1 : -1);
				refFrames[r] = vsapi->getFrameFilter(n + offset, d->super, frameCtx);
			}
		}
		for (int32_t i = 0; i < d->vi->format->numPlanes; i++) {
			pDst[i] = vsapi->getWritePtr(dst, i);
			nDstPitches[i] = vsapi->getStride(dst, i);
			pSrc[i] = vsapi->getReadPtr(src, i);
			nSrcPitches[i] = vsapi->getStride(src, i);
			for (int32_t r = 0; r < radius * 2; r++)
				if (isUsable[r]) {
					pRefs[i][r] = vsapi->getReadPtr(refFrames[r], i);
					nRefPitches[i][r] = vsapi->getStride(refFrames[r], i);
				}
		}
		const int32_t xSubUV = d->xSubUV;
		const int32_t ySubUV = d->ySubUV;
		const int32_t xRatioUV = d->bleh->xRatioUV;
		const int32_t yRatioUV = d->bleh->yRatioUV;
		const int32_t nBlkX = d->bleh->nBlkX;
		const int32_t nBlkY = d->bleh->nBlkY;
		const int32_t YUVplanes = d->YUVplanes;
		const int32_t dstTempPitch = d->dstTempPitch;
		const int32_t *nWidth = d->nWidth;
		const int32_t *nHeight = d->nHeight;
		const int32_t *nOverlapX = d->nOverlapX;
		const int32_t *nOverlapY = d->nOverlapY;
		const int32_t *nBlkSizeX = d->nBlkSizeX;
		const int32_t *nBlkSizeY = d->nBlkSizeY;
		const int32_t *nWidth_B = d->nWidth_B;
		const int32_t *nHeight_B = d->nHeight_B;
		const double *thSAD = d->thSAD;
		const double *nLimit = d->nLimit;
		MVGroupOfFrames *pRefGOF[radius * 2];
		for (int32_t r = 0; r < radius * 2; r++)
			pRefGOF[r] = new MVGroupOfFrames(d->nSuperLevels, nWidth[0], nHeight[0], d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, xRatioUV, yRatioUV);
		OverlapWindows *OverWins[3] = { d->OverWins[0], d->OverWins[1], d->OverWins[2] };
		uint8_t *DstTemp = nullptr;
		int32_t tmpBlockPitch = nBlkSizeX[0] * 4;
		uint8_t *tmpBlock = nullptr;
		if (nOverlapX[0] > 0 || nOverlapY[0] > 0) {
			DstTemp = new uint8_t[dstTempPitch * nHeight[0]];
			tmpBlock = new uint8_t[tmpBlockPitch * nBlkSizeY[0]];
		}
		MVPlane *pPlanes[3][radius * 2] = {};
		MVPlaneSet planes[3] = { YPLANE, UPLANE, VPLANE };
		for (int32_t r = 0; r < radius * 2; r++)
			if (isUsable[r]) {
				pRefGOF[r]->Update(YUVplanes, (uint8_t*)pRefs[0][r], nRefPitches[0][r], (uint8_t*)pRefs[1][r], nRefPitches[1][r], (uint8_t*)pRefs[2][r], nRefPitches[2][r]);
				for (int32_t plane = 0; plane < d->vi->format->numPlanes; plane++)
					if (YUVplanes & planes[plane])
						pPlanes[plane][r] = pRefGOF[r]->GetFrame(0)->GetPlane(planes[plane]);
			}
		pDstCur[0] = pDst[0];
		pDstCur[1] = pDst[1];
		pDstCur[2] = pDst[2];
		pSrcCur[0] = pSrc[0];
		pSrcCur[1] = pSrc[1];
		pSrcCur[2] = pSrc[2];
		for (int32_t plane = 0; plane < d->vi->format->numPlanes; plane++) {
			if (!d->process[plane]) {
				memcpy(pDstCur[plane], pSrcCur[plane], nSrcPitches[plane] * nHeight[plane]);
				continue;
			}
			if (nOverlapX[0] == 0 && nOverlapY[0] == 0) {
				for (int32_t by = 0; by < nBlkY; by++) {
					int32_t xx = 0;
					for (int32_t bx = 0; bx < nBlkX; bx++) {
						int32_t i = by * nBlkX + bx;
						const uint8_t *pointers[radius * 2];
						int32_t strides[radius * 2];
						double WSrc, WRefs[radius * 2];
						for (int32_t r = 0; r < radius * 2; r++)
							useBlock(pointers[r], strides[r], WRefs[r], isUsable[r], balls[r], i, pPlanes[plane][r], pSrcCur, xx, nSrcPitches, nLogPel, plane, xSubUV, ySubUV, thSAD);
						normalizeWeights<radius>(WSrc, WRefs);
						d->DEGRAIN[plane](pDstCur[plane] + xx, nDstPitches[plane], pSrcCur[plane] + xx, nSrcPitches[plane],
							pointers, strides,
							WSrc, WRefs);
						xx += nBlkSizeX[plane] * 4;
						if (bx == nBlkX - 1 && nWidth_B[0] < nWidth[0])
							vs_bitblt(pDstCur[plane] + nWidth_B[plane] * 4, nDstPitches[plane],
								pSrcCur[plane] + nWidth_B[plane] * 4, nSrcPitches[plane],
								(nWidth[plane] - nWidth_B[plane]) * 4, nBlkSizeY[plane]);
					}
					pDstCur[plane] += nBlkSizeY[plane] * (nDstPitches[plane]);
					pSrcCur[plane] += nBlkSizeY[plane] * (nSrcPitches[plane]);
					if (by == nBlkY - 1 && nHeight_B[0] < nHeight[0])
						vs_bitblt(pDstCur[plane], nDstPitches[plane],
							pSrcCur[plane], nSrcPitches[plane],
							nWidth[plane] * 4, nHeight[plane] - nHeight_B[plane]);
				}
			}
			else {
				uint8_t *pDstTemp = DstTemp;
				memset(pDstTemp, 0, dstTempPitch * nHeight_B[0]);
				for (int32_t by = 0; by < nBlkY; by++) {
					int32_t wby = ((by + nBlkY - 3) / (nBlkY - 2)) * 3;
					int32_t xx = 0;
					for (int32_t bx = 0; bx < nBlkX; bx++) {
						int32_t wbx = (bx + nBlkX - 3) / (nBlkX - 2);
						int32_t *winOver = OverWins[plane]->GetWindow(wby + wbx);
						int32_t i = by * nBlkX + bx;
						const uint8_t *pointers[radius * 2];
						int32_t strides[radius * 2];
						double WSrc, WRefs[radius * 2];
						for (int32_t r = 0; r < radius * 2; r++)
							useBlock(pointers[r], strides[r], WRefs[r], isUsable[r], balls[r], i, pPlanes[plane][r], pSrcCur, xx, nSrcPitches, nLogPel, plane, xSubUV, ySubUV, thSAD);
						normalizeWeights<radius>(WSrc, WRefs);
						d->DEGRAIN[plane](tmpBlock, tmpBlockPitch, pSrcCur[plane] + xx, nSrcPitches[plane],
							pointers, strides,
							WSrc, WRefs);
						d->OVERS[plane](pDstTemp + xx * 2, dstTempPitch, tmpBlock, tmpBlockPitch, winOver, nBlkSizeX[plane]);
						xx += (nBlkSizeX[plane] - nOverlapX[plane]) * 4;
					}
					pSrcCur[plane] += (nBlkSizeY[plane] - nOverlapY[plane]) * nSrcPitches[plane];
					pDstTemp += (nBlkSizeY[plane] - nOverlapY[plane]) * dstTempPitch;
				}
				d->ToPixels(pDst[plane], nDstPitches[plane], DstTemp, dstTempPitch, nWidth_B[plane], nHeight_B[plane]);
				if (nWidth_B[0] < nWidth[0])
					vs_bitblt(pDst[plane] + nWidth_B[plane] * 4, nDstPitches[plane],
						pSrc[plane] + nWidth_B[plane] * 4, nSrcPitches[plane],
						(nWidth[plane] - nWidth_B[plane]) * 4, nHeight_B[plane]);
				if (nHeight_B[0] < nHeight[0])
					vs_bitblt(pDst[plane] + nDstPitches[plane] * nHeight_B[plane], nDstPitches[plane],
						pSrc[plane] + nSrcPitches[plane] * nHeight_B[plane], nSrcPitches[plane],
						nWidth[plane] * 4, nHeight[plane] - nHeight_B[plane]);
			}
			if (nLimit[plane] < 1.)
				d->LimitChanges(pDst[plane], nDstPitches[plane],
					pSrc[plane], nSrcPitches[plane],
					nWidth[plane], nHeight[plane], nLimit[plane]);
		}
		if (tmpBlock)
			delete[] tmpBlock;
		if (DstTemp)
			delete[] DstTemp;
		for (int32_t r = 0; r < radius * 2; r++) {
			delete pRefGOF[r];
			if (refFrames[r])
				vsapi->freeFrame(refFrames[r]);
			delete balls[r];
		}
		vsapi->freeFrame(src);
		return dst;
	}
	return nullptr;
}

template <int32_t radius>
static void VS_CC mvdegrainFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVDegrainData *d = reinterpret_cast<MVDegrainData *>(instanceData);
	if (d->nOverlapX[0] || d->nOverlapY[0]) {
		delete d->OverWins[0];
		if (d->vi->format->colorFamily != cmGray)
			delete d->OverWins[1];
	}
	for (int32_t r = 0; r < radius * 2; r++) {
		delete d->mvClips[r];
		vsapi->freeNode(d->vectors[r]);
	}
	vsapi->freeNode(d->super);
	vsapi->freeNode(d->node);
	delete d->bleh;
	delete d;
}

template <int32_t radius>
static void selectFunctions(MVDegrainData *d) {
	const int32_t xRatioUV = d->bleh->xRatioUV;
	const int32_t yRatioUV = d->bleh->yRatioUV;
	const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
	const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
	OverlapsFunction overs[33][33];
	DenoiseFunction degs[33][33];
	overs[2][2] = Overlaps_C<2, 2, double, float>;
	degs[2][2] = Degrain_C<radius, 2, 2, float>;
	overs[2][4] = Overlaps_C<2, 4, double, float>;
	degs[2][4] = Degrain_C<radius, 2, 4, float>;
	overs[4][2] = Overlaps_C<4, 2, double, float>;
	degs[4][2] = Degrain_C<radius, 4, 2, float>;
	overs[4][4] = Overlaps_C<4, 4, double, float>;
	degs[4][4] = Degrain_C<radius, 4, 4, float>;
	overs[4][8] = Overlaps_C<4, 8, double, float>;
	degs[4][8] = Degrain_C<radius, 4, 8, float>;
	overs[8][1] = Overlaps_C<8, 1, double, float>;
	degs[8][1] = Degrain_C<radius, 8, 1, float>;
	overs[8][2] = Overlaps_C<8, 2, double, float>;
	degs[8][2] = Degrain_C<radius, 8, 2, float>;
	overs[8][4] = Overlaps_C<8, 4, double, float>;
	degs[8][4] = Degrain_C<radius, 8, 4, float>;
	overs[8][8] = Overlaps_C<8, 8, double, float>;
	degs[8][8] = Degrain_C<radius, 8, 8, float>;
	overs[8][16] = Overlaps_C<8, 16, double, float>;
	degs[8][16] = Degrain_C<radius, 8, 16, float>;
	overs[16][1] = Overlaps_C<16, 1, double, float>;
	degs[16][1] = Degrain_C<radius, 16, 1, float>;
	overs[16][2] = Overlaps_C<16, 2, double, float>;
	degs[16][2] = Degrain_C<radius, 16, 2, float>;
	overs[16][4] = Overlaps_C<16, 4, double, float>;
	degs[16][4] = Degrain_C<radius, 16, 4, float>;
	overs[16][8] = Overlaps_C<16, 8, double, float>;
	degs[16][8] = Degrain_C<radius, 16, 8, float>;
	overs[16][16] = Overlaps_C<16, 16, double, float>;
	degs[16][16] = Degrain_C<radius, 16, 16, float>;
	overs[16][32] = Overlaps_C<16, 32, double, float>;
	degs[16][32] = Degrain_C<radius, 16, 32, float>;
	overs[32][8] = Overlaps_C<32, 8, double, float>;
	degs[32][8] = Degrain_C<radius, 32, 8, float>;
	overs[32][16] = Overlaps_C<32, 16, double, float>;
	degs[32][16] = Degrain_C<radius, 32, 16, float>;
	overs[32][32] = Overlaps_C<32, 32, double, float>;
	degs[32][32] = Degrain_C<radius, 32, 32, float>;
	d->LimitChanges = LimitChanges_C<float>;
	d->ToPixels = ToPixels<double, float>;
	d->OVERS[0] = overs[nBlkSizeX][nBlkSizeY];
	d->DEGRAIN[0] = degs[nBlkSizeX][nBlkSizeY];
	d->OVERS[1] = d->OVERS[2] = overs[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
	d->DEGRAIN[1] = d->DEGRAIN[2] = degs[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
}

template <int32_t radius>
static void VS_CC mvdegrainCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	std::string filter = "Degrain";
	filter.append(std::to_string(radius));
	MVDegrainData d;
	MVDegrainData *data;
	int err;
	const int m = vsapi->propNumElements(in, "thsad");
	const int n = vsapi->propNumElements(in, "limit");
	for (int i = 0; i < 3; ++i) {
		if (m == -1)
			d.thSAD[0] = d.thSAD[1] = d.thSAD[2] = 400.;
		else
			if (i < m)
				d.thSAD[i] = vsapi->propGetFloat(in, "thsad", i, nullptr);
			else
				d.thSAD[i] = d.thSAD[i - 1];
		if (n == -1)
			d.nLimit[0] = d.nLimit[1] = d.nLimit[2] = 1.;
		else
			if (i < n) {
				d.nLimit[i] = vsapi->propGetFloat(in, "limit", i, nullptr);
				if (d.nLimit[i] < 0. || d.nLimit[i] > 1.) {
					vsapi->setError(out, (filter + ": limit must be between 0.0 and " + std::to_string(1.0) + " (inclusive).").c_str());
					vsapi->freeNode(d.super);
					vsapi->freeNode(d.node);
					for (int32_t r = 0; r < radius * 2; ++r) {
						vsapi->freeNode(d.vectors[r]);
						delete d.mvClips[r];
					}
					delete d.bleh;
					return;
				}
			}
			else
				d.nLimit[i] = d.nLimit[i - 1];
	}
	int32_t plane = int64ToIntS(vsapi->propGetInt(in, "plane", 0, &err));
	if (err)
		plane = 4;
	d.nSCD1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.nSCD1 = MV_DEFAULT_SCD1;
	d.nSCD2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.nSCD2 = MV_DEFAULT_SCD2;
	if (plane < 0 || plane > 4) {
		vsapi->setError(out, (filter + ": plane must be between 0 and 4 (inclusive).").c_str());
		return;
	}
	int32_t planes[5] = { YPLANE, UPLANE, VPLANE, UVPLANES, YUVPLANES };
	d.YUVplanes = planes[plane];
	d.super = vsapi->propGetNode(in, "super", 0, nullptr);
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, (filter + ": failed to retrieve first frame from super clip. Error message: " + errorMsg).c_str());
		vsapi->freeNode(d.super);
		return;
	}
	const VSMap *props = vsapi->getFramePropsRO(evil);
	int32_t evil_err[6];
	int32_t nHeightS = int64ToIntS(vsapi->propGetInt(props, "Super_height", 0, &evil_err[0]));
	d.nSuperHPad = int64ToIntS(vsapi->propGetInt(props, "Super_hpad", 0, &evil_err[1]));
	d.nSuperVPad = int64ToIntS(vsapi->propGetInt(props, "Super_vpad", 0, &evil_err[2]));
	d.nSuperPel = int64ToIntS(vsapi->propGetInt(props, "Super_pel", 0, &evil_err[3]));
	d.nSuperModeYUV = int64ToIntS(vsapi->propGetInt(props, "Super_modeyuv", 0, &evil_err[4]));
	d.nSuperLevels = int64ToIntS(vsapi->propGetInt(props, "Super_levels", 0, &evil_err[5]));
	vsapi->freeFrame(evil);
	for (int32_t i = 0; i < 6; i++)
		if (evil_err[i]) {
			vsapi->setError(out, (filter + ": required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?").c_str());
			vsapi->freeNode(d.super);
			return;
		}
	d.vectors[Backward1] = vsapi->propGetNode(in, "mvbw", 0, nullptr);
	d.vectors[Forward1] = vsapi->propGetNode(in, "mvfw", 0, nullptr);
	d.vectors[Backward2] = vsapi->propGetNode(in, "mvbw2", 0, &err);
	d.vectors[Forward2] = vsapi->propGetNode(in, "mvfw2", 0, &err);
	d.vectors[Backward3] = vsapi->propGetNode(in, "mvbw3", 0, &err);
	d.vectors[Forward3] = vsapi->propGetNode(in, "mvfw3", 0, &err);
	d.vectors[Backward4] = vsapi->propGetNode(in, "mvbw4", 0, &err);
	d.vectors[Forward4] = vsapi->propGetNode(in, "mvfw4", 0, &err);
	d.vectors[Backward5] = vsapi->propGetNode(in, "mvbw5", 0, &err);
	d.vectors[Forward5] = vsapi->propGetNode(in, "mvfw5", 0, &err);
	d.vectors[Backward6] = vsapi->propGetNode(in, "mvbw6", 0, &err);
	d.vectors[Forward6] = vsapi->propGetNode(in, "mvfw6", 0, &err);
	d.vectors[Backward7] = vsapi->propGetNode(in, "mvbw7", 0, &err);
	d.vectors[Forward7] = vsapi->propGetNode(in, "mvfw7", 0, &err);
	d.vectors[Backward8] = vsapi->propGetNode(in, "mvbw8", 0, &err);
	d.vectors[Forward8] = vsapi->propGetNode(in, "mvfw8", 0, &err);
	d.vectors[Backward9] = vsapi->propGetNode(in, "mvbw9", 0, &err);
	d.vectors[Forward9] = vsapi->propGetNode(in, "mvfw9", 0, &err);
	d.vectors[Backward10] = vsapi->propGetNode(in, "mvbw10", 0, &err);
	d.vectors[Forward10] = vsapi->propGetNode(in, "mvfw10", 0, &err);
	d.vectors[Backward11] = vsapi->propGetNode(in, "mvbw11", 0, &err);
	d.vectors[Forward11] = vsapi->propGetNode(in, "mvfw11", 0, &err);
	d.vectors[Backward12] = vsapi->propGetNode(in, "mvbw12", 0, &err);
	d.vectors[Forward12] = vsapi->propGetNode(in, "mvfw12", 0, &err);
	d.vectors[Backward13] = vsapi->propGetNode(in, "mvbw13", 0, &err);
	d.vectors[Forward13] = vsapi->propGetNode(in, "mvfw13", 0, &err);
	d.vectors[Backward14] = vsapi->propGetNode(in, "mvbw14", 0, &err);
	d.vectors[Forward14] = vsapi->propGetNode(in, "mvfw14", 0, &err);
	d.vectors[Backward15] = vsapi->propGetNode(in, "mvbw15", 0, &err);
	d.vectors[Forward15] = vsapi->propGetNode(in, "mvfw15", 0, &err);
	d.vectors[Backward16] = vsapi->propGetNode(in, "mvbw16", 0, &err);
	d.vectors[Forward16] = vsapi->propGetNode(in, "mvfw16", 0, &err);
	d.vectors[Backward17] = vsapi->propGetNode(in, "mvbw17", 0, &err);
	d.vectors[Forward17] = vsapi->propGetNode(in, "mvfw17", 0, &err);
	d.vectors[Backward18] = vsapi->propGetNode(in, "mvbw18", 0, &err);
	d.vectors[Forward18] = vsapi->propGetNode(in, "mvfw18", 0, &err);
	d.vectors[Backward19] = vsapi->propGetNode(in, "mvbw19", 0, &err);
	d.vectors[Forward19] = vsapi->propGetNode(in, "mvfw19", 0, &err);
	d.vectors[Backward20] = vsapi->propGetNode(in, "mvbw20", 0, &err);
	d.vectors[Forward20] = vsapi->propGetNode(in, "mvfw20", 0, &err);
	d.vectors[Backward21] = vsapi->propGetNode(in, "mvbw21", 0, &err);
	d.vectors[Forward21] = vsapi->propGetNode(in, "mvfw21", 0, &err);
	d.vectors[Backward22] = vsapi->propGetNode(in, "mvbw22", 0, &err);
	d.vectors[Forward22] = vsapi->propGetNode(in, "mvfw22", 0, &err);
	d.vectors[Backward23] = vsapi->propGetNode(in, "mvbw23", 0, &err);
	d.vectors[Forward23] = vsapi->propGetNode(in, "mvfw23", 0, &err);
	d.vectors[Backward24] = vsapi->propGetNode(in, "mvbw24", 0, &err);
	d.vectors[Forward24] = vsapi->propGetNode(in, "mvfw24", 0, &err);
	for (int32_t r = 0; r < radius * 2; r++) {
		try {
			d.mvClips[r] = new MVClipDicks(d.vectors[r], d.nSCD1, d.nSCD2, vsapi);
		}
		catch (MVException &e) {
			vsapi->setError(out, (filter + ": " + e.what()).c_str());

			vsapi->freeNode(d.super);

			for (int32_t rr = 0; rr < radius * 2; rr++)
				vsapi->freeNode(d.vectors[rr]);

			for (int32_t rr = 0; rr < r; rr++)
				delete d.mvClips[rr];

			return;
		}
	}
	try {
		if (!d.mvClips[Backward1]->IsBackward())
			throw MVException("mvbw must be generated with isb=True.");
		if (d.mvClips[Forward1]->IsBackward())
			throw MVException("mvfw must be generated with isb=False.");
		if (radius > 1) {
			if (!d.mvClips[Backward2]->IsBackward())
				throw MVException("mvbw2 must be generated with isb=True.");
			if (d.mvClips[Forward2]->IsBackward())
				throw MVException("mvfw2 must be generated with isb=False.");
			if (d.mvClips[Backward2]->GetDeltaFrame() <= d.mvClips[Backward1]->GetDeltaFrame())
				throw MVException("mvbw2 must have greater delta than mvbw.");
			if (d.mvClips[Forward2]->GetDeltaFrame() <= d.mvClips[Forward1]->GetDeltaFrame())
				throw MVException("mvfw2 must have greater delta than mvfw.");
		}
		if (radius > 2) {
			if (!d.mvClips[Backward3]->IsBackward())
				throw MVException("mvbw3 must be generated with isb=True.");
			if (d.mvClips[Forward3]->IsBackward())
				throw MVException("mvfw3 must be generated with isb=False.");
			if (d.mvClips[Backward3]->GetDeltaFrame() <= d.mvClips[Backward2]->GetDeltaFrame())
				throw MVException("mvbw3 must have greater delta than mvbw2.");
			if (d.mvClips[Forward3]->GetDeltaFrame() <= d.mvClips[Forward2]->GetDeltaFrame())
				throw MVException("mvfw3 must have greater delta than mvfw2.");
		}
		if (radius > 3) {
			if (!d.mvClips[Backward4]->IsBackward())
				throw MVException("mvbw4 must be generated with isb=True.");
			if (d.mvClips[Forward4]->IsBackward())
				throw MVException("mvfw4 must be generated with isb=False.");
			if (d.mvClips[Backward4]->GetDeltaFrame() <= d.mvClips[Backward3]->GetDeltaFrame())
				throw MVException("mvbw4 must have greater delta than mvbw3.");
			if (d.mvClips[Forward4]->GetDeltaFrame() <= d.mvClips[Forward3]->GetDeltaFrame())
				throw MVException("mvfw4 must have greater delta than mvfw3.");
		}
		if (radius > 4) {
			if (!d.mvClips[Backward5]->IsBackward())
				throw MVException("mvbw5 must be generated with isb=True.");
			if (d.mvClips[Forward5]->IsBackward())
				throw MVException("mvfw5 must be generated with isb=False.");
			if (d.mvClips[Backward5]->GetDeltaFrame() <= d.mvClips[Backward4]->GetDeltaFrame())
				throw MVException("mvbw5 must have greater delta than mvbw4.");
			if (d.mvClips[Forward5]->GetDeltaFrame() <= d.mvClips[Forward4]->GetDeltaFrame())
				throw MVException("mvfw5 must have greater delta than mvfw4.");
		}
		if (radius > 5) {
			if (!d.mvClips[Backward6]->IsBackward())
				throw MVException("mvbw6 must be generated with isb=True.");
			if (d.mvClips[Forward6]->IsBackward())
				throw MVException("mvfw6 must be generated with isb=False.");
			if (d.mvClips[Backward6]->GetDeltaFrame() <= d.mvClips[Backward5]->GetDeltaFrame())
				throw MVException("mvbw6 must have greater delta than mvbw5.");
			if (d.mvClips[Forward6]->GetDeltaFrame() <= d.mvClips[Forward5]->GetDeltaFrame())
				throw MVException("mvfw6 must have greater delta than mvfw5.");
		}
		if (radius > 6) {
			if (!d.mvClips[Backward7]->IsBackward())
				throw MVException("mvbw7 must be generated with isb=True.");
			if (d.mvClips[Forward7]->IsBackward())
				throw MVException("mvfw7 must be generated with isb=False.");
			if (d.mvClips[Backward7]->GetDeltaFrame() <= d.mvClips[Backward6]->GetDeltaFrame())
				throw MVException("mvbw7 must have greater delta than mvbw6.");
			if (d.mvClips[Forward7]->GetDeltaFrame() <= d.mvClips[Forward6]->GetDeltaFrame())
				throw MVException("mvfw7 must have greater delta than mvfw6.");
		}
		if (radius > 7) {
			if (!d.mvClips[Backward8]->IsBackward())
				throw MVException("mvbw8 must be generated with isb=True.");
			if (d.mvClips[Forward8]->IsBackward())
				throw MVException("mvfw8 must be generated with isb=False.");
			if (d.mvClips[Backward8]->GetDeltaFrame() <= d.mvClips[Backward7]->GetDeltaFrame())
				throw MVException("mvbw8 must have greater delta than mvbw7.");
			if (d.mvClips[Forward8]->GetDeltaFrame() <= d.mvClips[Forward7]->GetDeltaFrame())
				throw MVException("mvfw8 must have greater delta than mvfw7.");
		}
		if (radius > 8) {
			if (!d.mvClips[Backward9]->IsBackward())
				throw MVException("mvbw9 must be generated with isb=True.");
			if (d.mvClips[Forward9]->IsBackward())
				throw MVException("mvfw9 must be generated with isb=False.");
			if (d.mvClips[Backward9]->GetDeltaFrame() <= d.mvClips[Backward8]->GetDeltaFrame())
				throw MVException("mvbw9 must have greater delta than mvbw8.");
			if (d.mvClips[Forward9]->GetDeltaFrame() <= d.mvClips[Forward8]->GetDeltaFrame())
				throw MVException("mvfw9 must have greater delta than mvfw8.");
		}
		if (radius > 9) {
			if (!d.mvClips[Backward10]->IsBackward())
				throw MVException("mvbw10 must be generated with isb=True.");
			if (d.mvClips[Forward10]->IsBackward())
				throw MVException("mvfw10 must be generated with isb=False.");
			if (d.mvClips[Backward10]->GetDeltaFrame() <= d.mvClips[Backward9]->GetDeltaFrame())
				throw MVException("mvbw10 must have greater delta than mvbw9.");
			if (d.mvClips[Forward10]->GetDeltaFrame() <= d.mvClips[Forward9]->GetDeltaFrame())
				throw MVException("mvfw10 must have greater delta than mvfw9.");
		}
		if (radius > 10) {
			if (!d.mvClips[Backward11]->IsBackward())
				throw MVException("mvbw11 must be generated with isb=True.");
			if (d.mvClips[Forward11]->IsBackward())
				throw MVException("mvfw11 must be generated with isb=False.");
			if (d.mvClips[Backward11]->GetDeltaFrame() <= d.mvClips[Backward10]->GetDeltaFrame())
				throw MVException("mvbw11 must have greater delta than mvbw10.");
			if (d.mvClips[Forward11]->GetDeltaFrame() <= d.mvClips[Forward10]->GetDeltaFrame())
				throw MVException("mvfw11 must have greater delta than mvfw10.");
		}
		if (radius > 11) {
			if (!d.mvClips[Backward12]->IsBackward())
				throw MVException("mvbw12 must be generated with isb=True.");
			if (d.mvClips[Forward12]->IsBackward())
				throw MVException("mvfw12 must be generated with isb=False.");
			if (d.mvClips[Backward12]->GetDeltaFrame() <= d.mvClips[Backward11]->GetDeltaFrame())
				throw MVException("mvbw12 must have greater delta than mvbw11.");
			if (d.mvClips[Forward12]->GetDeltaFrame() <= d.mvClips[Forward11]->GetDeltaFrame())
				throw MVException("mvfw12 must have greater delta than mvfw11.");
		}
		if (radius > 12) {
			if (!d.mvClips[Backward13]->IsBackward())
				throw MVException("mvbw13 must be generated with isb=True.");
			if (d.mvClips[Forward13]->IsBackward())
				throw MVException("mvfw13 must be generated with isb=False.");
			if (d.mvClips[Backward13]->GetDeltaFrame() <= d.mvClips[Backward12]->GetDeltaFrame())
				throw MVException("mvbw13 must have greater delta than mvbw12.");
			if (d.mvClips[Forward13]->GetDeltaFrame() <= d.mvClips[Forward12]->GetDeltaFrame())
				throw MVException("mvfw13 must have greater delta than mvfw12.");
		}
		if (radius > 13) {
			if (!d.mvClips[Backward14]->IsBackward())
				throw MVException("mvbw14 must be generated with isb=True.");
			if (d.mvClips[Forward14]->IsBackward())
				throw MVException("mvfw14 must be generated with isb=False.");
			if (d.mvClips[Backward14]->GetDeltaFrame() <= d.mvClips[Backward13]->GetDeltaFrame())
				throw MVException("mvbw14 must have greater delta than mvbw13.");
			if (d.mvClips[Forward14]->GetDeltaFrame() <= d.mvClips[Forward13]->GetDeltaFrame())
				throw MVException("mvfw14 must have greater delta than mvfw13.");
		}
		if (radius > 14) {
			if (!d.mvClips[Backward15]->IsBackward())
				throw MVException("mvbw15 must be generated with isb=True.");
			if (d.mvClips[Forward15]->IsBackward())
				throw MVException("mvfw15 must be generated with isb=False.");
			if (d.mvClips[Backward15]->GetDeltaFrame() <= d.mvClips[Backward14]->GetDeltaFrame())
				throw MVException("mvbw15 must have greater delta than mvbw14.");
			if (d.mvClips[Forward15]->GetDeltaFrame() <= d.mvClips[Forward14]->GetDeltaFrame())
				throw MVException("mvfw15 must have greater delta than mvfw14.");
		}
		if (radius > 15) {
			if (!d.mvClips[Backward16]->IsBackward())
				throw MVException("mvbw16 must be generated with isb=True.");
			if (d.mvClips[Forward16]->IsBackward())
				throw MVException("mvfw16 must be generated with isb=False.");
			if (d.mvClips[Backward16]->GetDeltaFrame() <= d.mvClips[Backward15]->GetDeltaFrame())
				throw MVException("mvbw16 must have greater delta than mvbw15.");
			if (d.mvClips[Forward16]->GetDeltaFrame() <= d.mvClips[Forward15]->GetDeltaFrame())
				throw MVException("mvfw16 must have greater delta than mvfw15.");
		}
		if (radius > 16) {
			if (!d.mvClips[Backward17]->IsBackward())
				throw MVException("mvbw17 must be generated with isb=True.");
			if (d.mvClips[Forward17]->IsBackward())
				throw MVException("mvfw17 must be generated with isb=False.");
			if (d.mvClips[Backward17]->GetDeltaFrame() <= d.mvClips[Backward16]->GetDeltaFrame())
				throw MVException("mvbw17 must have greater delta than mvbw16.");
			if (d.mvClips[Forward17]->GetDeltaFrame() <= d.mvClips[Forward16]->GetDeltaFrame())
				throw MVException("mvfw17 must have greater delta than mvfw16.");
		}
		if (radius > 17) {
			if (!d.mvClips[Backward18]->IsBackward())
				throw MVException("mvbw18 must be generated with isb=True.");
			if (d.mvClips[Forward18]->IsBackward())
				throw MVException("mvfw18 must be generated with isb=False.");
			if (d.mvClips[Backward18]->GetDeltaFrame() <= d.mvClips[Backward17]->GetDeltaFrame())
				throw MVException("mvbw18 must have greater delta than mvbw17.");
			if (d.mvClips[Forward18]->GetDeltaFrame() <= d.mvClips[Forward17]->GetDeltaFrame())
				throw MVException("mvfw18 must have greater delta than mvfw17.");
		}
		if (radius > 18) {
			if (!d.mvClips[Backward19]->IsBackward())
				throw MVException("mvbw19 must be generated with isb=True.");
			if (d.mvClips[Forward19]->IsBackward())
				throw MVException("mvfw19 must be generated with isb=False.");
			if (d.mvClips[Backward19]->GetDeltaFrame() <= d.mvClips[Backward18]->GetDeltaFrame())
				throw MVException("mvbw19 must have greater delta than mvbw18.");
			if (d.mvClips[Forward19]->GetDeltaFrame() <= d.mvClips[Forward18]->GetDeltaFrame())
				throw MVException("mvfw19 must have greater delta than mvfw18.");
		}
		if (radius > 19) {
			if (!d.mvClips[Backward20]->IsBackward())
				throw MVException("mvbw20 must be generated with isb=True.");
			if (d.mvClips[Forward20]->IsBackward())
				throw MVException("mvfw20 must be generated with isb=False.");
			if (d.mvClips[Backward20]->GetDeltaFrame() <= d.mvClips[Backward19]->GetDeltaFrame())
				throw MVException("mvbw20 must have greater delta than mvbw19.");
			if (d.mvClips[Forward20]->GetDeltaFrame() <= d.mvClips[Forward19]->GetDeltaFrame())
				throw MVException("mvfw20 must have greater delta than mvfw19.");
		}
		if (radius > 20) {
			if (!d.mvClips[Backward21]->IsBackward())
				throw MVException("mvbw21 must be generated with isb=True.");
			if (d.mvClips[Forward21]->IsBackward())
				throw MVException("mvfw21 must be generated with isb=False.");
			if (d.mvClips[Backward21]->GetDeltaFrame() <= d.mvClips[Backward20]->GetDeltaFrame())
				throw MVException("mvbw21 must have greater delta than mvbw20.");
			if (d.mvClips[Forward21]->GetDeltaFrame() <= d.mvClips[Forward20]->GetDeltaFrame())
				throw MVException("mvfw21 must have greater delta than mvfw20.");
		}
		if (radius > 21) {
			if (!d.mvClips[Backward22]->IsBackward())
				throw MVException("mvbw22 must be generated with isb=True.");
			if (d.mvClips[Forward22]->IsBackward())
				throw MVException("mvfw22 must be generated with isb=False.");
			if (d.mvClips[Backward22]->GetDeltaFrame() <= d.mvClips[Backward21]->GetDeltaFrame())
				throw MVException("mvbw22 must have greater delta than mvbw21.");
			if (d.mvClips[Forward22]->GetDeltaFrame() <= d.mvClips[Forward21]->GetDeltaFrame())
				throw MVException("mvfw22 must have greater delta than mvfw21.");
		}
		if (radius > 22) {
			if (!d.mvClips[Backward23]->IsBackward())
				throw MVException("mvbw23 must be generated with isb=True.");
			if (d.mvClips[Forward23]->IsBackward())
				throw MVException("mvfw23 must be generated with isb=False.");
			if (d.mvClips[Backward23]->GetDeltaFrame() <= d.mvClips[Backward22]->GetDeltaFrame())
				throw MVException("mvbw23 must have greater delta than mvbw22.");
			if (d.mvClips[Forward23]->GetDeltaFrame() <= d.mvClips[Forward22]->GetDeltaFrame())
				throw MVException("mvfw23 must have greater delta than mvfw22.");
		}
		if (radius > 23) {
			if (!d.mvClips[Backward24]->IsBackward())
				throw MVException("mvbw24 must be generated with isb=True.");
			if (d.mvClips[Forward24]->IsBackward())
				throw MVException("mvfw24 must be generated with isb=False.");
			if (d.mvClips[Backward24]->GetDeltaFrame() <= d.mvClips[Backward23]->GetDeltaFrame())
				throw MVException("mvbw24 must have greater delta than mvbw23.");
			if (d.mvClips[Forward24]->GetDeltaFrame() <= d.mvClips[Forward23]->GetDeltaFrame())
				throw MVException("mvfw24 must have greater delta than mvfw23.");
		}
		d.bleh = new MVFilter(d.vectors[Forward1], filter.c_str(), vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, (filter + ": " + e.what()).c_str());
		vsapi->freeNode(d.super);
		for (int32_t r = 0; r < radius * 2; r++) {
			vsapi->freeNode(d.vectors[r]);
			delete d.mvClips[r];
		}
		return;
	}
	try {
		const char *vectorNames[48] = { "mvbw", "mvfw", "mvbw2", "mvfw2", "mvbw3", "mvfw3", "mvbw4", "mvfw4", "mvbw5", "mvfw5", "mvbw6", "mvfw6", "mvbw7", "mvfw7", "mvbw8", "mvfw8", "mvbw9", "mvfw9", "mvbw10", "mvfw10", "mvbw11", "mvfw11", "mvbw12", "mvfw12", "mvbw13", "mvfw13", "mvbw14", "mvfw14", "mvbw15", "mvfw15", "mvbw16", "mvfw16", "mvbw17", "mvfw17", "mvbw18", "mvfw18", "mvbw19", "mvfw19", "mvbw20", "mvfw20", "mvbw21", "mvfw21", "mvbw22", "mvfw22", "mvbw23", "mvfw23", "mvbw24", "mvfw24" };
		for (int32_t r = 0; r < radius * 2; r++)
			d.bleh->CheckSimilarity(d.mvClips[r], vectorNames[r]);
	}
	catch (MVException &e) {
		vsapi->setError(out, (filter + ": " + e.what()).c_str());
		vsapi->freeNode(d.super);
		for (int32_t r = 0; r < radius * 2; r++) {
			vsapi->freeNode(d.vectors[r]);
			delete d.mvClips[r];
		}
		delete d.bleh;
		return;
	}
	d.thSAD[0] = d.thSAD[0] * d.mvClips[Backward1]->GetThSCD1() / d.nSCD1;
	d.thSAD[1] = d.thSAD[2] = d.thSAD[1] * d.mvClips[Backward1]->GetThSCD1() / d.nSCD1;
	d.node = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = vsapi->getVideoInfo(d.node);
	const VSVideoInfo *supervi = vsapi->getVideoInfo(d.super);
	int32_t nSuperWidth = supervi->width;
	if (d.bleh->nHeight != nHeightS || d.bleh->nHeight != d.vi->height || d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2 || d.bleh->nWidth != d.vi->width) {
		vsapi->setError(out, (filter + ": wrong source or super clip frame size.").c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.node);
		for (int r = 0; r < radius * 2; ++r) {
			vsapi->freeNode(d.vectors[r]);
			delete d.mvClips[r];
		}
		delete d.bleh;
		return;
	}
	if (!isConstantFormat(d.vi) || d.vi->format->bitsPerSample < 32 || d.vi->format->sampleType != stFloat) {
		vsapi->setError(out, (filter + ": input clip must be single precision fp, with constant dimensions.").c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.node);
		for (int r = 0; r < radius * 2; ++r) {
			vsapi->freeNode(d.vectors[r]);
			delete d.mvClips[r];
		}
		delete d.bleh;
		return;
	}
	d.dstTempPitch = ((d.bleh->nWidth + 15) / 16) * 16 * 4 * 2;
	d.process[0] = d.vi->format->colorFamily == cmRGB ? true : !!(d.YUVplanes & YPLANE);
	d.process[1] = d.vi->format->colorFamily == cmRGB ? true : !!(d.YUVplanes & UPLANE & d.nSuperModeYUV);
	d.process[2] = d.vi->format->colorFamily == cmRGB ? true : !!(d.YUVplanes & VPLANE & d.nSuperModeYUV);
	d.xSubUV = d.vi->format->subSamplingW;
	d.ySubUV = d.vi->format->subSamplingH;
	d.nWidth[0] = d.bleh->nWidth;
	d.nWidth[1] = d.nWidth[2] = d.nWidth[0] >> d.xSubUV;
	d.nHeight[0] = d.bleh->nHeight;
	d.nHeight[1] = d.nHeight[2] = d.nHeight[0] >> d.ySubUV;
	d.nOverlapX[0] = d.bleh->nOverlapX;
	d.nOverlapX[1] = d.nOverlapX[2] = d.nOverlapX[0] >> d.xSubUV;
	d.nOverlapY[0] = d.bleh->nOverlapY;
	d.nOverlapY[1] = d.nOverlapY[2] = d.nOverlapY[0] >> d.ySubUV;
	d.nBlkSizeX[0] = d.bleh->nBlkSizeX;
	d.nBlkSizeX[1] = d.nBlkSizeX[2] = d.nBlkSizeX[0] >> d.xSubUV;
	d.nBlkSizeY[0] = d.bleh->nBlkSizeY;
	d.nBlkSizeY[1] = d.nBlkSizeY[2] = d.nBlkSizeY[0] >> d.ySubUV;
	d.nWidth_B[0] = d.bleh->nBlkX * (d.nBlkSizeX[0] - d.nOverlapX[0]) + d.nOverlapX[0];
	d.nWidth_B[1] = d.nWidth_B[2] = d.nWidth_B[0] >> d.xSubUV;
	d.nHeight_B[0] = d.bleh->nBlkY * (d.nBlkSizeY[0] - d.nOverlapY[0]) + d.nOverlapY[0];
	d.nHeight_B[1] = d.nHeight_B[2] = d.nHeight_B[0] >> d.ySubUV;
	if (d.nOverlapX[0] || d.nOverlapY[0]) {
		d.OverWins[0] = new OverlapWindows(d.nBlkSizeX[0], d.nBlkSizeY[0], d.nOverlapX[0], d.nOverlapY[0]);
		if (d.vi->format->colorFamily != cmGray) {
			d.OverWins[1] = new OverlapWindows(d.nBlkSizeX[1], d.nBlkSizeY[1], d.nOverlapX[1], d.nOverlapY[1]);
			d.OverWins[2] = d.OverWins[1];
		}
	}
	selectFunctions<radius>(&d);
	data = new MVDegrainData;
	*data = d;
	vsapi->createFilter(in, out, filter.c_str(), mvdegrainInit, mvdegrainGetFrame<radius>, mvdegrainFree<radius>, fmParallel, 0, data, core);
}

void mvdegrainsRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Degrain1",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<1>, 0, plugin);
	registerFunc("Degrain2",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<2>, 0, plugin);
	registerFunc("Degrain3",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<3>, 0, plugin);
	registerFunc("Degrain4",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<4>, 0, plugin);
	registerFunc("Degrain5",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<5>, 0, plugin);
	registerFunc("Degrain6",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<6>, 0, plugin);
	registerFunc("Degrain7",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<7>, 0, plugin);
	registerFunc("Degrain8",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<8>, 0, plugin);
	registerFunc("Degrain9",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<9>, 0, plugin);
	registerFunc("Degrain10",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<10>, 0, plugin);
	registerFunc("Degrain11",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<11>, 0, plugin);
	registerFunc("Degrain12",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<12>, 0, plugin);
	registerFunc("Degrain13",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<13>, 0, plugin);
	registerFunc("Degrain14",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<14>, 0, plugin);
	registerFunc("Degrain15",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<15>, 0, plugin);
	registerFunc("Degrain16",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<16>, 0, plugin);
	registerFunc("Degrain17",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<17>, 0, plugin);
	registerFunc("Degrain18",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<18>, 0, plugin);
	registerFunc("Degrain19",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"mvbw19:clip;"
		"mvfw19:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<19>, 0, plugin);
	registerFunc("Degrain20",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"mvbw19:clip;"
		"mvfw19:clip;"
		"mvbw20:clip;"
		"mvfw20:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<20>, 0, plugin);
	registerFunc("Degrain21",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"mvbw19:clip;"
		"mvfw19:clip;"
		"mvbw20:clip;"
		"mvfw20:clip;"
		"mvbw21:clip;"
		"mvfw21:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<21>, 0, plugin);
	registerFunc("Degrain22",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"mvbw19:clip;"
		"mvfw19:clip;"
		"mvbw20:clip;"
		"mvfw20:clip;"
		"mvbw21:clip;"
		"mvfw21:clip;"
		"mvbw22:clip;"
		"mvfw22:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<22>, 0, plugin);
	registerFunc("Degrain23",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"mvbw19:clip;"
		"mvfw19:clip;"
		"mvbw20:clip;"
		"mvfw20:clip;"
		"mvbw21:clip;"
		"mvfw21:clip;"
		"mvbw22:clip;"
		"mvfw22:clip;"
		"mvbw23:clip;"
		"mvfw23:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<23>, 0, plugin);
	registerFunc("Degrain24",
		"clip:clip;"
		"super:clip;"
		"mvbw:clip;"
		"mvfw:clip;"
		"mvbw2:clip;"
		"mvfw2:clip;"
		"mvbw3:clip;"
		"mvfw3:clip;"
		"mvbw4:clip;"
		"mvfw4:clip;"
		"mvbw5:clip;"
		"mvfw5:clip;"
		"mvbw6:clip;"
		"mvfw6:clip;"
		"mvbw7:clip;"
		"mvfw7:clip;"
		"mvbw8:clip;"
		"mvfw8:clip;"
		"mvbw9:clip;"
		"mvfw9:clip;"
		"mvbw10:clip;"
		"mvfw10:clip;"
		"mvbw11:clip;"
		"mvfw11:clip;"
		"mvbw12:clip;"
		"mvfw12:clip;"
		"mvbw13:clip;"
		"mvfw13:clip;"
		"mvbw14:clip;"
		"mvfw14:clip;"
		"mvbw15:clip;"
		"mvfw15:clip;"
		"mvbw16:clip;"
		"mvfw16:clip;"
		"mvbw17:clip;"
		"mvfw17:clip;"
		"mvbw18:clip;"
		"mvfw18:clip;"
		"mvbw19:clip;"
		"mvfw19:clip;"
		"mvbw20:clip;"
		"mvfw20:clip;"
		"mvbw21:clip;"
		"mvfw21:clip;"
		"mvbw22:clip;"
		"mvfw22:clip;"
		"mvbw23:clip;"
		"mvfw23:clip;"
		"mvbw24:clip;"
		"mvfw24:clip;"
		"thsad:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate<24>, 0, plugin);
}
