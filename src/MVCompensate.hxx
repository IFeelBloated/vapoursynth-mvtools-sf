#pragma once
#include "Include/VapourSynth.h"
#include "Include/VSHelper.h"
#include "MVFilter.hpp"
#include "CopyCode.hpp"
#include "Overlap.h"
#include "MVClip.hpp"
#include "MVFrame.h"
#include "SADFunctions.hpp"

struct MVCompensateData {
	VSNodeRef *node;
	const VSVideoInfo *vi;
	const VSVideoInfo *supervi;
	VSNodeRef *super;
	VSNodeRef *vectors;
	bool scBehavior;
	double thSAD;
	bool fields;
	int32_t time256;
	double nSCD1;
	double nSCD2;
	bool tff;
	int32_t tffexists;
	MVClipDicks *mvClip;
	MVFilter *bleh;
	int32_t nSuperHPad;
	int32_t nSuperVPad;
	int32_t nSuperPel;
	int32_t nSuperModeYUV;
	int32_t nSuperLevels;
	int32_t dstTempPitch;
	int32_t dstTempPitchUV;
	OverlapWindows *OverWins;
	OverlapWindows *OverWinsUV;
	OverlapsFunction OVERSLUMA;
	OverlapsFunction OVERSCHROMA;
	COPYFunction BLITLUMA;
	COPYFunction BLITCHROMA;
	ToPixelsFunction ToPixels;
};

static void VS_CC mvcompensateInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	MVCompensateData *d = reinterpret_cast<MVCompensateData *>(*instanceData);
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mvcompensateGetFrame(int32_t n, int32_t activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
	MVCompensateData *d = reinterpret_cast<MVCompensateData *>(*instanceData);
	if (activationReason == arInitial) {
		int32_t off, nref;
		if (d->mvClip->GetDeltaFrame() > 0) {
			off = (d->mvClip->IsBackward()) ? 1 : -1;
			off *= d->mvClip->GetDeltaFrame();
			nref = n + off;
		}
		else
			nref = -d->mvClip->GetDeltaFrame();
		vsapi->requestFrameFilter(n, d->vectors, frameCtx);
		if (nref < n && nref >= 0)
			vsapi->requestFrameFilter(nref, d->super, frameCtx);
		vsapi->requestFrameFilter(n, d->super, frameCtx);
		if (nref >= n && (!d->vi->numFrames || nref < d->vi->numFrames))
			vsapi->requestFrameFilter(nref, d->super, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->super, frameCtx);
		VSFrameRef *dst = vsapi->newVideoFrame(d->vi->format, d->vi->width, d->vi->height, src, core);
		uint8_t *pDst[3], *pDstCur[3];
		const uint8_t *pRef[3];
		int32_t nDstPitches[3], nRefPitches[3];
		const uint8_t *pSrc[3], *pSrcCur[3];
		int32_t nSrcPitches[3];
		uint8_t *pDstTemp;
		uint8_t *pDstTempU;
		uint8_t *pDstTempV;
		int32_t blx, bly;
		const VSFrameRef *mvn = vsapi->getFrameFilter(n, d->vectors, frameCtx);
		MVClipBalls balls(d->mvClip, vsapi);
		balls.Update(mvn);
		vsapi->freeFrame(mvn);
		int32_t off, nref;
		if (d->mvClip->GetDeltaFrame() > 0) {
			off = (d->mvClip->IsBackward()) ? 1 : -1;
			off *= d->mvClip->GetDeltaFrame();
			nref = n + off;
		}
		else
			nref = -d->mvClip->GetDeltaFrame();
		const int32_t nWidth = d->bleh->nWidth;
		const int32_t nHeight = d->bleh->nHeight;
		const int32_t xRatioUV = d->bleh->xRatioUV;
		const int32_t yRatioUV = d->bleh->yRatioUV;
		const int32_t nOverlapX = d->bleh->nOverlapX;
		const int32_t nOverlapY = d->bleh->nOverlapY;
		const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
		const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
		const int32_t nBlkX = d->bleh->nBlkX;
		const int32_t nBlkY = d->bleh->nBlkY;
		const double thSAD = d->thSAD;
		const int32_t dstTempPitch = d->dstTempPitch;
		const int32_t dstTempPitchUV = d->dstTempPitchUV;
		const int32_t nSuperModeYUV = d->nSuperModeYUV;
		const int32_t nPel = d->bleh->nPel;
		const int32_t nHPadding = d->bleh->nHPadding;
		const int32_t nVPadding = d->bleh->nVPadding;
		const int32_t scBehavior = d->scBehavior;
		const int32_t fields = d->fields;
		const int32_t time256 = d->time256;
		int32_t nWidth_B = nBlkX*(nBlkSizeX - nOverlapX) + nOverlapX;
		int32_t nHeight_B = nBlkY*(nBlkSizeY - nOverlapY) + nOverlapY;
		int32_t ySubUV = (yRatioUV == 2) ? 1 : 0;
		int32_t xSubUV = (xRatioUV == 2) ? 1 : 0;
		if (balls.IsUsable()) {
			const VSFrameRef *ref = vsapi->getFrameFilter(nref, d->super, frameCtx);
			for (int32_t i = 0; i < d->supervi->format->numPlanes; i++) {
				pDst[i] = vsapi->getWritePtr(dst, i);
				nDstPitches[i] = vsapi->getStride(dst, i);
				pSrc[i] = vsapi->getReadPtr(src, i);
				nSrcPitches[i] = vsapi->getStride(src, i);
				pRef[i] = vsapi->getReadPtr(ref, i);
				nRefPitches[i] = vsapi->getStride(ref, i);
			}
			MVGroupOfFrames *pRefGOF = new MVGroupOfFrames(d->nSuperLevels, nWidth, nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, nSuperModeYUV, xRatioUV, yRatioUV);
			MVGroupOfFrames *pSrcGOF = new MVGroupOfFrames(d->nSuperLevels, nWidth, nHeight, d->nSuperPel, d->nSuperHPad, d->nSuperVPad, nSuperModeYUV, xRatioUV, yRatioUV);
			pRefGOF->Update(nSuperModeYUV, (uint8_t*)pRef[0], nRefPitches[0], (uint8_t*)pRef[1], nRefPitches[1], (uint8_t*)pRef[2], nRefPitches[2]);
			pSrcGOF->Update(nSuperModeYUV, (uint8_t*)pSrc[0], nSrcPitches[0], (uint8_t*)pSrc[1], nSrcPitches[1], (uint8_t*)pSrc[2], nSrcPitches[2]);
			MVPlaneSet planes[3] = { YPLANE, UPLANE, VPLANE };
			MVPlane *pPlanes[3] = { 0 };
			MVPlane *pSrcPlanes[3] = { 0 };
			for (int32_t plane = 0; plane < d->supervi->format->numPlanes; ++plane) {
				pPlanes[plane] = pRefGOF->GetFrame(0)->GetPlane(planes[plane]);
				pSrcPlanes[plane] = pSrcGOF->GetFrame(0)->GetPlane(planes[plane]);
				pDstCur[plane] = pDst[plane];
				pSrcCur[plane] = pSrc[plane];
			}
			int32_t fieldShift = 0;
			if (fields && nPel > 1 && ((nref - n) % 2 != 0)) {
				int err;
				const VSMap *props = vsapi->getFramePropsRO(src);
				bool paritySrc = !!vsapi->propGetInt(props, "_Field", 0, &err); //child->GetParity(n);
				if (err && !d->tffexists) {
					vsapi->setFilterError("Compensate: _Field property not found in input frame. Therefore, you must pass tff argument.", frameCtx);
					delete pRefGOF;
					delete pSrcGOF;
					vsapi->freeFrame(src);
					vsapi->freeFrame(dst);
					vsapi->freeFrame(ref);
					return nullptr;
				}
				if (d->tffexists)
					paritySrc = !!(static_cast<int>(d->tff) ^ (n % 2));
				props = vsapi->getFramePropsRO(ref);
				bool parityRef = !!vsapi->propGetInt(props, "_Field", 0, &err);
				if (err && !d->tffexists) {
					vsapi->setFilterError("Compensate: _Field property not found in input frame. Therefore, you must pass tff argument.", frameCtx);
					delete pRefGOF;
					delete pSrcGOF;
					vsapi->freeFrame(src);
					vsapi->freeFrame(dst);
					vsapi->freeFrame(ref);
					return nullptr;
				}
				if (d->tffexists)
					parityRef = !!(static_cast<int>(d->tff) ^ (nref % 2));
				fieldShift = (paritySrc && !parityRef) ? nPel / 2 : ((parityRef && !paritySrc) ? -(nPel / 2) : 0);
			}
			if (nOverlapX == 0 && nOverlapY == 0) {
				for (int32_t by = 0; by<nBlkY; ++by) {
					int32_t xx = 0;
					for (int32_t bx = 0; bx<nBlkX; ++bx) {
						int32_t i = by*nBlkX + bx;
						auto &block = balls[0][i];
						blx = static_cast<int32_t>(block.GetX() * nPel + static_cast<int64_t>(block.GetMV().x) * time256 / 256);
						bly = static_cast<int32_t>(block.GetY() * nPel + static_cast<int64_t>(block.GetMV().y) * time256 / 256 + fieldShift);
						if (block.GetSAD() < thSAD) {
							d->BLITLUMA(pDstCur[0] + xx, nDstPitches[0], pPlanes[0]->GetPointer(blx, bly), pPlanes[0]->GetPitch());
							if (pPlanes[1]) d->BLITCHROMA(pDstCur[1] + (xx >> xSubUV), nDstPitches[1], pPlanes[1]->GetPointer(blx >> xSubUV, bly >> ySubUV), pPlanes[1]->GetPitch());
							if (pPlanes[2]) d->BLITCHROMA(pDstCur[2] + (xx >> xSubUV), nDstPitches[2], pPlanes[2]->GetPointer(blx >> xSubUV, bly >> ySubUV), pPlanes[2]->GetPitch());
						}
						else {
							int32_t blxsrc = bx * (nBlkSizeX)* nPel;
							int32_t blysrc = by * (nBlkSizeY)* nPel + fieldShift;
							d->BLITLUMA(pDstCur[0] + xx, nDstPitches[0], pSrcPlanes[0]->GetPointer(blxsrc, blysrc), pSrcPlanes[0]->GetPitch());
							if (pSrcPlanes[1]) d->BLITCHROMA(pDstCur[1] + (xx >> xSubUV), nDstPitches[1], pSrcPlanes[1]->GetPointer(blxsrc >> xSubUV, blysrc >> ySubUV), pSrcPlanes[1]->GetPitch());
							if (pSrcPlanes[2]) d->BLITCHROMA(pDstCur[2] + (xx >> xSubUV), nDstPitches[2], pSrcPlanes[2]->GetPointer(blxsrc >> xSubUV, blysrc >> ySubUV), pSrcPlanes[2]->GetPitch());
						}
						xx += (nBlkSizeX * 4);
					}
					pDstCur[0] += (nBlkSizeY)* (nDstPitches[0]);
					pSrcCur[0] += (nBlkSizeY)* (nSrcPitches[0]);
					if (nSuperModeYUV & UVPLANES) {
						pDstCur[1] += (nBlkSizeY >> ySubUV) * (nDstPitches[1]);
						pDstCur[2] += (nBlkSizeY >> ySubUV) * (nDstPitches[2]);
						pSrcCur[1] += (nBlkSizeY >> ySubUV) * (nSrcPitches[1]);
						pSrcCur[2] += (nBlkSizeY >> ySubUV) * (nSrcPitches[2]);
					}
				}
			}
			else {
				OverlapWindows *OverWins = d->OverWins;
				OverlapWindows *OverWinsUV = d->OverWinsUV;
				uint8_t *DstTemp = new uint8_t[dstTempPitch * nHeight];
				uint8_t *DstTempU = nullptr;
				uint8_t *DstTempV = nullptr;
				if (nSuperModeYUV & UVPLANES) {
					DstTempU = new uint8_t[dstTempPitchUV * nHeight];
					DstTempV = new uint8_t[dstTempPitchUV * nHeight];
				}
				pDstTemp = DstTemp;
				pDstTempU = DstTempU;
				pDstTempV = DstTempV;
				memset(DstTemp, 0, nHeight_B * dstTempPitch);
				if (pPlanes[1])
					memset(DstTempU, 0, (nHeight_B >> ySubUV) * dstTempPitchUV);
				if (pPlanes[2])
					memset(DstTempV, 0, (nHeight_B >> ySubUV) * dstTempPitchUV);
				for (int32_t by = 0; by<nBlkY; by++) {
					int32_t wby = ((by + nBlkY - 3) / (nBlkY - 2)) * 3;
					int32_t xx = 0;
					for (int32_t bx = 0; bx<nBlkX; bx++) {
						int32_t wbx = (bx + nBlkX - 3) / (nBlkX - 2);
						auto winOver = OverWins->GetWindow(wby + wbx);
						auto winOverUV = static_cast<double*>(nullptr);
						if (nSuperModeYUV & UVPLANES)
							winOverUV = OverWinsUV->GetWindow(wby + wbx);
						int32_t i = by*nBlkX + bx;
						auto &block = balls[0][i];
						blx = static_cast<int32_t>(block.GetX() * nPel + static_cast<int64_t>(block.GetMV().x) * time256 / 256);
						bly = static_cast<int32_t>(block.GetY() * nPel + static_cast<int64_t>(block.GetMV().y) * time256 / 256 + fieldShift);
						if (block.GetSAD() < thSAD) {
							d->OVERSLUMA(pDstTemp + xx * 2, dstTempPitch, pPlanes[0]->GetPointer(blx, bly), pPlanes[0]->GetPitch(), winOver, nBlkSizeX);
							if (pPlanes[1]) d->OVERSCHROMA(pDstTempU + (xx >> xSubUV) * 2, dstTempPitchUV, pPlanes[1]->GetPointer(blx >> xSubUV, bly >> ySubUV), pPlanes[1]->GetPitch(), winOverUV, nBlkSizeX >> xSubUV);
							if (pPlanes[2]) d->OVERSCHROMA(pDstTempV + (xx >> xSubUV) * 2, dstTempPitchUV, pPlanes[2]->GetPointer(blx >> xSubUV, bly >> ySubUV), pPlanes[2]->GetPitch(), winOverUV, nBlkSizeX >> xSubUV);
						}
						else {
							int32_t blxsrc = bx * (nBlkSizeX - nOverlapX) * nPel;
							int32_t blysrc = by * (nBlkSizeY - nOverlapY) * nPel + fieldShift;
							d->OVERSLUMA(pDstTemp + xx * 2, dstTempPitch, pSrcPlanes[0]->GetPointer(blxsrc, blysrc), pSrcPlanes[0]->GetPitch(), winOver, nBlkSizeX);
							if (pSrcPlanes[1]) d->OVERSCHROMA(pDstTempU + (xx >> xSubUV) * 2, dstTempPitchUV, pSrcPlanes[1]->GetPointer(blxsrc >> xSubUV, blysrc >> ySubUV), pSrcPlanes[1]->GetPitch(), winOverUV, nBlkSizeX >> xSubUV);
							if (pSrcPlanes[2]) d->OVERSCHROMA(pDstTempV + (xx >> xSubUV) * 2, dstTempPitchUV, pSrcPlanes[2]->GetPointer(blxsrc >> xSubUV, blysrc >> ySubUV), pSrcPlanes[2]->GetPitch(), winOverUV, nBlkSizeX >> xSubUV);
						}
						xx += (nBlkSizeX - nOverlapX) * 4;
					}
					pDstTemp += dstTempPitch*(nBlkSizeY - nOverlapY);
					pDstCur[0] += (nBlkSizeY - nOverlapY) * (nDstPitches[0]);
					pSrcCur[0] += (nBlkSizeY - nOverlapY) * (nSrcPitches[0]);
					if (nSuperModeYUV & UVPLANES) {
						pDstTempU += dstTempPitchUV*((nBlkSizeY - nOverlapY) >> ySubUV);
						pDstTempV += dstTempPitchUV*((nBlkSizeY - nOverlapY) >> ySubUV);
						pDstCur[1] += ((nBlkSizeY - nOverlapY) >> ySubUV) * (nDstPitches[1]);
						pDstCur[2] += ((nBlkSizeY - nOverlapY) >> ySubUV) * (nDstPitches[2]);
						pSrcCur[1] += ((nBlkSizeY - nOverlapY) >> ySubUV) * (nSrcPitches[1]);
						pSrcCur[2] += ((nBlkSizeY - nOverlapY) >> ySubUV) * (nSrcPitches[2]);
					}
				}
				d->ToPixels(pDst[0], nDstPitches[0], DstTemp, dstTempPitch, nWidth_B, nHeight_B);
				if (pPlanes[1])
					d->ToPixels(pDst[1], nDstPitches[1], DstTempU, dstTempPitchUV, nWidth_B >> xSubUV, nHeight_B >> ySubUV);
				if (pPlanes[2])
					d->ToPixels(pDst[2], nDstPitches[2], DstTempV, dstTempPitchUV, nWidth_B >> xSubUV, nHeight_B >> ySubUV);
				delete[] DstTemp;
				if (nSuperModeYUV & UVPLANES) {
					delete[] DstTempU;
					delete[] DstTempV;
				}
			}
			const uint8_t *scSrc[3] = { 0 };
			int32_t scPitches[3] = { 0 };
			for (int32_t i = 0; i < 3; i++) {
				if (scBehavior) {
					scSrc[i] = pSrc[i];
					scPitches[i] = nSrcPitches[i];
				}
				else {
					scSrc[i] = pRef[i];
					scPitches[i] = nRefPitches[i];
				}
			}
			if (nWidth_B < nWidth) {
				vs_bitblt(pDst[0] + nWidth_B * 4, nDstPitches[0],
					scSrc[0] + (nWidth_B + nHPadding) * 4 + nVPadding * scPitches[0], scPitches[0],
					(nWidth - nWidth_B) * 4, nHeight_B);
				if (pPlanes[1])
					vs_bitblt(pDst[1] + (nWidth_B >> xSubUV) * 4, nDstPitches[1],
						scSrc[1] + ((nWidth_B >> xSubUV) + (nHPadding >> xSubUV)) * 4 + (nVPadding >> ySubUV) * scPitches[1], scPitches[1],
						((nWidth - nWidth_B) >> xSubUV) * 4, nHeight_B >> ySubUV);
				if (pPlanes[2])
					vs_bitblt(pDst[2] + (nWidth_B >> xSubUV) * 4, nDstPitches[2],
						scSrc[2] + ((nWidth_B >> xSubUV) + (nHPadding >> xSubUV)) * 4 + (nVPadding >> ySubUV) * scPitches[2], scPitches[2],
						((nWidth - nWidth_B) >> xSubUV) * 4, nHeight_B >> ySubUV);
			}
			if (nHeight_B < nHeight) {
				vs_bitblt(pDst[0] + nHeight_B * nDstPitches[0], nDstPitches[0],
					scSrc[0] + nHPadding * 4 + (nHeight_B + nVPadding) * scPitches[0], scPitches[0],
					nWidth * 4, nHeight - nHeight_B);
				if (pPlanes[1])
					vs_bitblt(pDst[1] + (nHeight_B >> ySubUV) * nDstPitches[1], nDstPitches[1],
						scSrc[1] + nHPadding * 4 + ((nHeight_B + nVPadding) >> ySubUV) * scPitches[1], scPitches[1],
						(nWidth >> xSubUV) * 4, (nHeight - nHeight_B) >> ySubUV);
				if (pPlanes[2])
					vs_bitblt(pDst[2] + (nHeight_B >> ySubUV) * nDstPitches[2], nDstPitches[2],
						scSrc[2] + nHPadding * 4 + ((nHeight_B + nVPadding) >> ySubUV) * scPitches[2], scPitches[2],
						(nWidth >> xSubUV) * 4, (nHeight - nHeight_B) >> ySubUV);
			}
			delete pSrcGOF;
			delete pRefGOF;
			vsapi->freeFrame(ref);
		}
		else {
			if (!scBehavior && (nref < d->vi->numFrames || !d->vi->numFrames) && (nref >= 0)) {
				vsapi->freeFrame(src);
				src = vsapi->getFrameFilter(nref, d->super, frameCtx);
			}
			for (int32_t i = 0; i < d->supervi->format->numPlanes; i++) {
				pDst[i] = vsapi->getWritePtr(dst, i);
				nDstPitches[i] = vsapi->getStride(dst, i);
				pSrc[i] = vsapi->getReadPtr(src, i);
				nSrcPitches[i] = vsapi->getStride(src, i);
			}
			int32_t nOffset[3];
			nOffset[0] = nHPadding * 4 + nVPadding * nSrcPitches[0];
			nOffset[1] = nHPadding * 4 / xRatioUV + (nVPadding / yRatioUV) * nSrcPitches[1];
			nOffset[2] = nOffset[1];
			vs_bitblt(pDst[0], nDstPitches[0], pSrc[0] + nOffset[0], nSrcPitches[0], nWidth * 4, nHeight);
			if (nSuperModeYUV & UVPLANES) {
				vs_bitblt(pDst[1], nDstPitches[1], pSrc[1] + nOffset[1], nSrcPitches[1], nWidth * 4 / xRatioUV, nHeight / yRatioUV);
				vs_bitblt(pDst[2], nDstPitches[2], pSrc[2] + nOffset[2], nSrcPitches[2], nWidth * 4 / xRatioUV, nHeight / yRatioUV);
			}
		}
		vsapi->freeFrame(src);
		return dst;
	}
	return nullptr;
}

static void VS_CC mvcompensateFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	MVCompensateData *d = reinterpret_cast<MVCompensateData *>(instanceData);
	if (d->bleh->nOverlapX || d->bleh->nOverlapY) {
		delete d->OverWins;
		if (d->nSuperModeYUV & UVPLANES)
			delete d->OverWinsUV;
	}
	delete d->mvClip;
	delete d->bleh;
	vsapi->freeNode(d->super);
	vsapi->freeNode(d->vectors);
	vsapi->freeNode(d->node);
	delete d;
}

static void selectFunctions(MVCompensateData *d) {
	const int32_t xRatioUV = d->bleh->xRatioUV;
	const int32_t yRatioUV = d->bleh->yRatioUV;
	const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
	const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
	static OverlapsFunction overs[257][257];
	static COPYFunction copys[257][257];
	overs[2][2] = Overlaps_C<2, 2, double, float>;
	copys[2][2] = Copy_C<2, 2>;
	overs[2][4] = Overlaps_C<2, 4, double, float>;
	copys[2][4] = Copy_C<2, 4>;
	overs[4][2] = Overlaps_C<4, 2, double, float>;
	copys[4][2] = Copy_C<4, 2>;
	overs[4][4] = Overlaps_C<4, 4, double, float>;
	copys[4][4] = Copy_C<4, 4>;
	overs[4][8] = Overlaps_C<4, 8, double, float>;
	copys[4][8] = Copy_C<4, 8>;
	overs[8][1] = Overlaps_C<8, 1, double, float>;
	copys[8][1] = Copy_C<8, 1>;
	overs[8][2] = Overlaps_C<8, 2, double, float>;
	copys[8][2] = Copy_C<8, 2>;
	overs[8][4] = Overlaps_C<8, 4, double, float>;
	copys[8][4] = Copy_C<8, 4>;
	overs[8][8] = Overlaps_C<8, 8, double, float>;
	copys[8][8] = Copy_C<8, 8>;
	overs[8][16] = Overlaps_C<8, 16, double, float>;
	copys[8][16] = Copy_C<8, 16>;
	overs[16][1] = Overlaps_C<16, 1, double, float>;
	copys[16][1] = Copy_C<16, 1>;
	overs[16][2] = Overlaps_C<16, 2, double, float>;
	copys[16][2] = Copy_C<16, 2>;
	overs[16][4] = Overlaps_C<16, 4, double, float>;
	copys[16][4] = Copy_C<16, 4>;
	overs[16][8] = Overlaps_C<16, 8, double, float>;
	copys[16][8] = Copy_C<16, 8>;
	overs[16][16] = Overlaps_C<16, 16, double, float>;
	copys[16][16] = Copy_C<16, 16>;
	overs[16][32] = Overlaps_C<16, 32, double, float>;
	copys[16][32] = Copy_C<16, 32>;
	overs[32][8] = Overlaps_C<32, 8, double, float>;
	copys[32][8] = Copy_C<32, 8>;
	overs[32][16] = Overlaps_C<32, 16, double, float>;
	copys[32][16] = Copy_C<32, 16>;
	overs[32][32] = Overlaps_C<32, 32, double, float>;
	copys[32][32] = Copy_C<32, 32>;
	overs[32][64] = Overlaps_C<32, 64, double, float>;
	copys[32][64] = Copy_C<32, 64>;
	overs[64][16] = Overlaps_C<64, 16, double, float>;
	copys[64][16] = Copy_C<64, 16>;
	overs[64][32] = Overlaps_C<64, 32, double, float>;
	copys[64][32] = Copy_C<64, 32>;
	overs[64][64] = Overlaps_C<64, 64, double, float>;
	copys[64][64] = Copy_C<64, 64>;
	overs[64][128] = Overlaps_C<64, 128, double, float>;
	copys[64][128] = Copy_C<64, 128>;
	overs[128][32] = Overlaps_C<128, 32, double, float>;
	copys[128][32] = Copy_C<128, 32>;
	overs[128][64] = Overlaps_C<128, 64, double, float>;
	copys[128][64] = Copy_C<128, 64>;
	overs[128][128] = Overlaps_C<128, 128, double, float>;
	copys[128][128] = Copy_C<128, 128>;
	overs[128][256] = Overlaps_C<128, 256, double, float>;
	copys[128][256] = Copy_C<128, 256>;
	overs[256][64] = Overlaps_C<256, 64, double, float>;
	copys[256][64] = Copy_C<256, 64>;
	overs[256][128] = Overlaps_C<256, 128, double, float>;
	copys[256][128] = Copy_C<256, 128>;
	overs[256][256] = Overlaps_C<256, 256, double, float>;
	copys[256][256] = Copy_C<256, 256>;
	d->ToPixels = ToPixels<double, float>;
	d->OVERSLUMA = overs[nBlkSizeX][nBlkSizeY];
	d->BLITLUMA = copys[nBlkSizeX][nBlkSizeY];
	d->OVERSCHROMA = overs[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
	d->BLITCHROMA = copys[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
}

auto CreateCompensate(auto in, auto out, auto vsapi) {
	MVCompensateData d;
	int err;
	d.scBehavior = !!vsapi->propGetInt(in, "scbehavior", 0, &err);
	if (err)
		d.scBehavior = 1;
	d.thSAD = vsapi->propGetFloat(in, "thsad", 0, &err);
	if (err)
		d.thSAD = 10000.;
	d.fields = !!vsapi->propGetInt(in, "fields", 0, &err);
	double time = vsapi->propGetFloat(in, "time", 0, &err);
	if (err)
		time = 100.;
	d.nSCD1 = vsapi->propGetFloat(in, "thscd1", 0, &err);
	if (err)
		d.nSCD1 = MV_DEFAULT_SCD1;
	d.nSCD2 = vsapi->propGetFloat(in, "thscd2", 0, &err);
	if (err)
		d.nSCD2 = MV_DEFAULT_SCD2;
	d.tff = !!vsapi->propGetInt(in, "tff", 0, &err);
	d.tffexists = !err;
	if (time < 0. || time > 100.) {
		vsapi->setError(out, "Compensate: time must be between 0.0 and 100.0 (inclusive).");
		return d;
	}
	d.super = vsapi->propGetNode(in, "super", 0, nullptr);
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, d.super, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, std::string("Compensate: failed to retrieve first frame from super clip. Error message: ").append(errorMsg).c_str());
		vsapi->freeNode(d.super);
		return d;
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
			vsapi->setError(out, "Compensate: required properties not found in first frame of super clip. Maybe clip didn't come from mv.Super? Was the first frame trimmed away?");
			vsapi->freeNode(d.super);
			return d;
		}
	d.vectors = vsapi->propGetNode(in, "vectors", 0, nullptr);
	try {
		d.mvClip = new MVClipDicks(d.vectors, d.nSCD1, d.nSCD2, vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Compensate: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		return d;
	}
	try {
		d.bleh = new MVFilter(d.vectors, "Compensate", vsapi);
	}
	catch (MVException &e) {
		vsapi->setError(out, std::string("Compensate: ").append(e.what()).c_str());
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		delete d.mvClip;
		return d;
	}
	if (d.fields && d.bleh->nPel < 2) {
		vsapi->setError(out, "Compensate: fields option requires pel > 1.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		delete d.mvClip;
		delete d.bleh;
		return d;
	}
	d.thSAD = d.thSAD * d.mvClip->GetThSCD1() / d.nSCD1;
	d.node = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = vsapi->getVideoInfo(d.node);
	d.dstTempPitch = ((d.bleh->nWidth + 15) / 16) * 16 * 4 * 2;
	d.dstTempPitchUV = (((d.bleh->nWidth / d.bleh->xRatioUV) + 15) / 16) * 16 * 4 * 2;
	d.supervi = vsapi->getVideoInfo(d.super);
	int32_t nSuperWidth = d.supervi->width;
	if (d.bleh->nHeight != nHeightS || d.bleh->nHeight != d.vi->height || d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2 || d.bleh->nWidth != d.vi->width) {
		vsapi->setError(out, "Compensate: wrong source or super clip frame size.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		vsapi->freeNode(d.node);
		delete d.mvClip;
		delete d.bleh;
		return d;
	}
	if (!isConstantFormat(d.vi) || d.vi->format->bitsPerSample < 32 || d.vi->format->sampleType != stFloat) {
		vsapi->setError(out, "Compensate: input clip must be single precision fp, with constant dimensions.");
		vsapi->freeNode(d.super);
		vsapi->freeNode(d.vectors);
		vsapi->freeNode(d.node);
		delete d.mvClip;
		delete d.bleh;
		return d;
	}
	if (d.bleh->nOverlapX || d.bleh->nOverlapY) {
		d.OverWins = new OverlapWindows(d.bleh->nBlkSizeX, d.bleh->nBlkSizeY, d.bleh->nOverlapX, d.bleh->nOverlapY);
		if (d.nSuperModeYUV & UVPLANES)
			d.OverWinsUV = new OverlapWindows(d.bleh->nBlkSizeX / d.bleh->xRatioUV, d.bleh->nBlkSizeY / d.bleh->yRatioUV, d.bleh->nOverlapX / d.bleh->xRatioUV, d.bleh->nOverlapY / d.bleh->yRatioUV);
	}
	d.time256 = static_cast<int32_t>(time * 256. / 100.);
	selectFunctions(&d);
	return d;
}

static void VS_CC mvcompensateCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	auto args = ArgumentList{ in };
	auto Core = VaporCore{ core };
	auto clip = Clip{};
	auto vectors = Clip{};
	auto cclip = Clip{};
	clip = args["clip"];
	vectors = args["vectors"];
	if (args["cclip"].Exists())
		cclip = args["cclip"];
	else
		cclip = clip;
	auto Eval = [&](auto&& vec) {
		auto CreateArgumentMap = [&]() {
			auto Map = vsapi->createMap();
			for (auto x : Range{ vsapi->propNumKeys(in) }) {
				auto ItemSrc = ReadonlyItem{ in, vsapi->propGetKey(in, x) };
				auto ItemDst = WritableItem{ Map, vsapi->propGetKey(in, x) };
				if (ItemSrc.Type() == VSPropTypes::ptNode) {
					auto val = Clip{};
					val = ItemSrc;
					ItemDst = val;
				}
				else if (ItemSrc.Type() == VSPropTypes::ptInt) {
					auto val = 0_i64;
					val = ItemSrc;
					ItemDst = val;
				}
				else if (ItemSrc.Type() == VSPropTypes::ptFloat) {
					auto val = 0.;
					val = ItemSrc;
					ItemDst = val;
				}
			}
			auto v = WritableItem{ Map, "vectors" };
			v.Erase();
			v = vec;
			return Map;
		};
		auto argMap = CreateArgumentMap();
		auto evalMap = vsapi->createMap();
		auto data = new MVCompensateData{ CreateCompensate(argMap, out, vsapi) };
		if (vsapi->getError(out) != nullptr) {
			delete data;
			vsapi->freeMap(argMap);
			vsapi->freeMap(evalMap);
			return Clip{};
		}
		vsapi->createFilter(argMap, evalMap, "Compensate", mvcompensateInit, mvcompensateGetFrame, mvcompensateFree, fmParallel, 0, data, core);
		auto comp = vsapi->propGetNode(evalMap, "clip", 0, nullptr);
		vsapi->freeMap(argMap);
		vsapi->freeMap(evalMap);
		return Clip{ comp };
	};
	if (clip.FrameCount == vectors.FrameCount) {
		auto data = new MVCompensateData{ CreateCompensate(in, out, vsapi) };
		if (vsapi->getError(out) != nullptr) {
			delete data;
			return;
		}
		vsapi->createFilter(in, out, "Compensate", mvcompensateInit, mvcompensateGetFrame, mvcompensateFree, fmParallel, 0, data, core);
	}
	else {
		auto radius = vectors.FrameCount / clip.FrameCount / 2;
		auto comps = std::vector<Clip>{};
		comps.reserve(2 * radius + 1);
		for (auto x : Range{ radius }) {
			auto comp = Eval(Core["std"]["SelectEvery"]("clip", vectors, "cycle", 2 * radius, "offsets", x));
			if (comp.ContainsVideoReference() == false)
				return;
			comps.push_back(std::move(comp));
		}
		comps.push_back(cclip);
		for (auto x : Range{ radius, 2 * radius })
			comps.push_back(Eval(Core["std"]["SelectEvery"]("clip", vectors, "cycle", 2 * radius, "offsets", x)));
		auto compmulti = Core["std"]["Interleave"]("clips", comps);
		VaporGlobals::API->propSetNode(out, "clip", compmulti.VideoNode, VSPropAppendMode::paAppend);
	}
}

void mvcompensateRegister(VSRegisterFunction registerFunc, VSPlugin *plugin) {
	registerFunc("Compensate",
		"clip:clip;"
		"super:clip;"
		"vectors:clip;"
		"cclip:clip:opt;"
		"scbehavior:int:opt;"
		"thsad:float:opt;"
		"fields:int:opt;"
		"time:float:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		"tff:int:opt;"
		, mvcompensateCreate, 0, plugin);
}
