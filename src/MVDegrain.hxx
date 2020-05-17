#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include <limits>
#include "MVFrame.h"
#include "SADFunctions.hpp"
#include "Include/VapourSynth.h"
#include "Include/VSHelper.h"
#include "MVClip.hpp"
#include "MVFilter.hpp"
#include "MVInterface.h"
#include "Overlap.h"
#include "Include/Interface.hxx"

using DenoiseFunction = auto(*)(int, uint8_t*, int32_t, const uint8_t*, int32_t, const uint8_t**, const int32_t*, double, const double*)->void;

template<int32_t blockWidth, int32_t blockHeight, typename PixelType>
void Degrain_C(auto radius, uint8_t* pDst8, int32_t nDstPitch, const uint8_t* pSrc8, int32_t nSrcPitch, const uint8_t** pRefs8, const int32_t* nRefPitches, double WSrc, const double* WRefs) {
	for (int32_t y = 0; y < blockHeight; y++) {
		for (int32_t x = 0; x < blockWidth; x++) {
			const PixelType* pSrc = (const PixelType*)pSrc8;
			PixelType* pDst = (PixelType*)pDst8;
			double sum = pSrc[x] * WSrc;
			for (int32_t r = 0; r < radius * 2; r++) {
				const PixelType* pRef = (const PixelType*)pRefs8[r];
				sum += pRef[x] * WRefs[r];
			}
			pDst[x] = static_cast<PixelType>(sum / 256);
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
		for (int32_t r = 0; r < radius * 2; r++)
			pRefs8[r] += nRefPitches[r];
	}
}

using LimitFunction = auto(*)(uint8_t*, intptr_t, const uint8_t*, intptr_t, intptr_t, intptr_t, double)->void;

template <typename PixelType>
static void LimitChanges_C(uint8_t* pDst8, intptr_t nDstPitch, const uint8_t* pSrc8, intptr_t nSrcPitch, intptr_t nWidth, intptr_t nHeight, double nLimit) {
	for (int32_t h = 0; h < nHeight; h++) {
		for (int32_t i = 0; i < nWidth; i++) {
			const PixelType* pSrc = (const PixelType*)pSrc8;
			PixelType* pDst = (PixelType*)pDst8;
			pDst[i] = (PixelType)VSMIN(VSMAX(pDst[i], (pSrc[i] - nLimit)), (pSrc[i] + nLimit));
		}
		pDst8 += nDstPitch;
		pSrc8 += nSrcPitch;
	}
}

inline double DegrainWeight(double thSAD, double blockSAD) {
	if (blockSAD >= thSAD)
		return 0.;
	return (thSAD - blockSAD) * (thSAD + blockSAD) * 256 / (thSAD * thSAD + blockSAD * blockSAD);
}

inline void useBlock(const uint8_t*& p, int32_t& np, double& WRef, bool isUsable, const MVClipBalls& mvclip, int32_t i, const MVPlane* pPlane, const uint8_t** pSrcCur, int32_t xx, const int32_t* nSrcPitch, int32_t nLogPel, int32_t plane, int32_t xSubUV, int32_t ySubUV, const double* thSAD) {
	if (isUsable) {
		auto& block = mvclip[0][i];
		int32_t blx = (block.GetX() << nLogPel) + block.GetMV().x;
		int32_t bly = (block.GetY() << nLogPel) + block.GetMV().y;
		p = pPlane->GetPointer(plane ? blx >> xSubUV : blx, plane ? bly >> ySubUV : bly);
		np = pPlane->GetPitch();
		double blockSAD = block.GetSAD();
		WRef = DegrainWeight(thSAD[plane], blockSAD);
	}
	else {
		p = pSrcCur[plane] + xx;
		np = nSrcPitch[plane];
		WRef = 0.;
	}
}

static inline void normalizeWeights(auto radius, double& WSrc, double* WRefs) {
	WSrc = 256.;
	double WSum = WSrc + 1.;
	for (int32_t r = 0; r < radius * 2; r++)
		WSum += WRefs[r];
	for (int32_t r = 0; r < radius * 2; r++) {
		WRefs[r] = WRefs[r] * 256 / WSum;
		WSrc -= WRefs[r];
	}
}

struct MVDegrainData {
	self(node, Clip{});
	self(super, Clip{});
	self(vectors, std::vector<Clip>{});
	self(radius, 0);
	self(thSAD, std::vector<std::array<double, 3>>{});
	int32_t YUVplanes;
	double nLimit[3];
	double nSCD1;
	double nSCD2;
	self(mvClips, std::vector<MVClipDicks>{});
	MVFilter* bleh;
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
	OverlapWindows* OverWins[3];
	template<typename T>
	auto CreateArray() {
		auto vec = std::vector<T>{};
		vec.resize(radius * 2);
		return vec;
	}
	template<typename T>
	auto CreateFrameArray() {
		auto vec = std::vector<T>{};
		vec.resize(radius * 2);
		return std::array{ vec,vec,vec };
	}
};

static void VS_CC mvdegrainInit(VSMap* in, VSMap* out, void** instanceData, VSNode* node, VSCore* core, const VSAPI* vsapi) {
	MVDegrainData* d = reinterpret_cast<MVDegrainData*>(*instanceData);
	auto vi = d->node.ExposeVideoInfo();
	vsapi->setVideoInfo(&vi, 1, node);
}

static const VSFrameRef* VS_CC mvdegrainGetFrame(int32_t n, int32_t activationReason, void** instanceData, void** frameData, VSFrameContext* frameCtx, VSCore* core, const VSAPI* vsapi) {
	MVDegrainData* d = reinterpret_cast<MVDegrainData*>(*instanceData);
	if (activationReason == arInitial) {
		for (auto& v : d->vectors)
			v.RequestFrame(n, frameCtx);
		for (auto r : Range{ d->radius })
			if (auto offF = -1 * d->mvClips[2 * r + 1].GetDeltaFrame(); n + offF >= 0)
				d->super.RequestFrame(n + offF, frameCtx);
		for (auto r : Range{ d->radius })
			if (auto offB = d->mvClips[2 * r].GetDeltaFrame(); n + offB < d->node.numFrames || !d->node.numFrames)
				d->super.RequestFrame(n + offB, frameCtx);
		d->node.RequestFrame(n, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrameRef* src = vsapi->getFrameFilter(n, d->node.VideoNode, frameCtx);
		VSFrameRef* dst = vsapi->newVideoFrame(d->node.format, d->node.width, d->node.height, src, core);
		uint8_t* pDst[3], * pDstCur[3];
		const uint8_t* pSrcCur[3];
		const uint8_t* pSrc[3];
		auto pRefs = d->CreateFrameArray<const uint8_t*>();
		int32_t nDstPitches[3], nSrcPitches[3];
		auto nRefPitches = d->CreateFrameArray<int32_t>();
		auto isUsable = d->CreateArray<bool>();
		int32_t nLogPel = (d->bleh->nPel == 4) ? 2 : (d->bleh->nPel == 2) ? 1 : 0;
		auto balls = d->CreateArray<MVClipBalls*>();
		auto refFrames = d->CreateArray<const VSFrameRef*>();
		for (auto& x : refFrames)
			x = nullptr;
		for (int32_t r = 0; r < d->radius * 2; r++) {
			const VSFrameRef* frame = vsapi->getFrameFilter(n, d->vectors[r].VideoNode, frameCtx);
			balls[r] = new MVClipBalls{ &d->mvClips[r], vsapi };
			balls[r]->Update(frame);
			isUsable[r] = balls[r]->IsUsable();
			vsapi->freeFrame(frame);
			if (isUsable[r]) {
				int32_t offset = d->mvClips[r].GetDeltaFrame() * (d->mvClips[r].IsBackward() ? 1 : -1);
				refFrames[r] = vsapi->getFrameFilter(n + offset, d->super.VideoNode, frameCtx);
			}
		}
		for (int32_t i = 0; i < d->node.numPlanes; i++) {
			pDst[i] = vsapi->getWritePtr(dst, i);
			nDstPitches[i] = vsapi->getStride(dst, i);
			pSrc[i] = vsapi->getReadPtr(src, i);
			nSrcPitches[i] = vsapi->getStride(src, i);
			for (int32_t r = 0; r < d->radius * 2; r++)
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
		const int32_t* nWidth = d->nWidth;
		const int32_t* nHeight = d->nHeight;
		const int32_t* nOverlapX = d->nOverlapX;
		const int32_t* nOverlapY = d->nOverlapY;
		const int32_t* nBlkSizeX = d->nBlkSizeX;
		const int32_t* nBlkSizeY = d->nBlkSizeY;
		const int32_t* nWidth_B = d->nWidth_B;
		const int32_t* nHeight_B = d->nHeight_B;
		const double* nLimit = d->nLimit;
		auto pRefGOF = d->CreateArray<MVGroupOfFrames*>();
		for (int32_t r = 0; r < d->radius * 2; r++)
			pRefGOF[r] = new MVGroupOfFrames(d->nSuperLevels, nWidth[0], nHeight[0], d->nSuperPel, d->nSuperHPad, d->nSuperVPad, d->nSuperModeYUV, xRatioUV, yRatioUV);
		OverlapWindows* OverWins[3] = { d->OverWins[0], d->OverWins[1], d->OverWins[2] };
		uint8_t* DstTemp = nullptr;
		int32_t tmpBlockPitch = nBlkSizeX[0] * 4;
		uint8_t* tmpBlock = nullptr;
		if (nOverlapX[0] > 0 || nOverlapY[0] > 0) {
			DstTemp = new uint8_t[dstTempPitch * nHeight[0]];
			tmpBlock = new uint8_t[tmpBlockPitch * nBlkSizeY[0]];
		}
		auto pPlanes = d->CreateFrameArray<MVPlane*>();
		MVPlaneSet planes[3] = { YPLANE, UPLANE, VPLANE };
		for (int32_t r = 0; r < d->radius * 2; r++)
			if (isUsable[r]) {
				pRefGOF[r]->Update(YUVplanes, (uint8_t*)pRefs[0][r], nRefPitches[0][r], (uint8_t*)pRefs[1][r], nRefPitches[1][r], (uint8_t*)pRefs[2][r], nRefPitches[2][r]);
				for (int32_t plane = 0; plane < d->node.numPlanes; plane++)
					if (YUVplanes & planes[plane])
						pPlanes[plane][r] = pRefGOF[r]->GetFrame(0)->GetPlane(planes[plane]);
			}
		pDstCur[0] = pDst[0];
		pDstCur[1] = pDst[1];
		pDstCur[2] = pDst[2];
		pSrcCur[0] = pSrc[0];
		pSrcCur[1] = pSrc[1];
		pSrcCur[2] = pSrc[2];
		for (int32_t plane = 0; plane < d->node.numPlanes; plane++) {
			if (!d->process[plane]) {
				memcpy(pDstCur[plane], pSrcCur[plane], nSrcPitches[plane] * nHeight[plane]);
				continue;
			}
			if (nOverlapX[0] == 0 && nOverlapY[0] == 0) {
				for (int32_t by = 0; by < nBlkY; by++) {
					int32_t xx = 0;
					for (int32_t bx = 0; bx < nBlkX; bx++) {
						int32_t i = by * nBlkX + bx;
						auto pointers = d->CreateArray<const uint8_t*>();
						auto strides = d->CreateArray<int32_t>();
						double WSrc;
						auto WRefs = d->CreateArray<double>();
						for (int32_t r = 0; r < d->radius * 2; r++)
							useBlock(pointers[r], strides[r], WRefs[r], isUsable[r], balls[r][0], i, pPlanes[plane][r], pSrcCur, xx, nSrcPitches, nLogPel, plane, xSubUV, ySubUV, d->thSAD[r].data());
						normalizeWeights(d->radius, WSrc, WRefs.data());
						d->DEGRAIN[plane](d->radius, pDstCur[plane] + xx, nDstPitches[plane], pSrcCur[plane] + xx, nSrcPitches[plane],
							pointers.data(), strides.data(),
							WSrc, WRefs.data());
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
				uint8_t* pDstTemp = DstTemp;
				memset(pDstTemp, 0, dstTempPitch * nHeight_B[0]);
				for (int32_t by = 0; by < nBlkY; by++) {
					int32_t wby = ((by + nBlkY - 3) / (nBlkY - 2)) * 3;
					int32_t xx = 0;
					for (int32_t bx = 0; bx < nBlkX; bx++) {
						int32_t wbx = (bx + nBlkX - 3) / (nBlkX - 2);
						auto winOver = OverWins[plane]->GetWindow(wby + wbx);
						int32_t i = by * nBlkX + bx;
						auto pointers = d->CreateArray<const uint8_t*>();
						auto strides = d->CreateArray<int32_t>();
						double WSrc;
						auto WRefs = d->CreateArray<double>();
						for (int32_t r = 0; r < d->radius * 2; r++)
							useBlock(pointers[r], strides[r], WRefs[r], isUsable[r], balls[r][0], i, pPlanes[plane][r], pSrcCur, xx, nSrcPitches, nLogPel, plane, xSubUV, ySubUV, d->thSAD[r].data());
						normalizeWeights(d->radius, WSrc, WRefs.data());
						d->DEGRAIN[plane](d->radius, tmpBlock, tmpBlockPitch, pSrcCur[plane] + xx, nSrcPitches[plane],
							pointers.data(), strides.data(),
							WSrc, WRefs.data());
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
			if (nLimit[plane] < std::numeric_limits<double>::infinity())
				d->LimitChanges(pDst[plane], nDstPitches[plane],
					pSrc[plane], nSrcPitches[plane],
					nWidth[plane], nHeight[plane], nLimit[plane]);
		}
		if (tmpBlock)
			delete[] tmpBlock;
		if (DstTemp)
			delete[] DstTemp;
		for (int32_t r = 0; r < d->radius * 2; r++) {
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

static void VS_CC mvdegrainFree(void* instanceData, VSCore* core, const VSAPI* vsapi) {
	MVDegrainData* d = reinterpret_cast<MVDegrainData*>(instanceData);
	if (d->nOverlapX[0] || d->nOverlapY[0]) {
		delete d->OverWins[0];
		if (d->node.colorFamily != cmGray)
			delete d->OverWins[1];
	}
	delete d->bleh;
	delete d;
}

static void selectFunctions(MVDegrainData* d) {
	const int32_t xRatioUV = d->bleh->xRatioUV;
	const int32_t yRatioUV = d->bleh->yRatioUV;
	const int32_t nBlkSizeX = d->bleh->nBlkSizeX;
	const int32_t nBlkSizeY = d->bleh->nBlkSizeY;
	static OverlapsFunction overs[257][257];
	static DenoiseFunction degs[257][257];
	overs[2][2] = Overlaps_C<2, 2, double, float>;
	degs[2][2] = Degrain_C<2, 2, float>;
	overs[2][4] = Overlaps_C<2, 4, double, float>;
	degs[2][4] = Degrain_C<2, 4, float>;
	overs[4][2] = Overlaps_C<4, 2, double, float>;
	degs[4][2] = Degrain_C<4, 2, float>;
	overs[4][4] = Overlaps_C<4, 4, double, float>;
	degs[4][4] = Degrain_C<4, 4, float>;
	overs[4][8] = Overlaps_C<4, 8, double, float>;
	degs[4][8] = Degrain_C<4, 8, float>;
	overs[8][1] = Overlaps_C<8, 1, double, float>;
	degs[8][1] = Degrain_C<8, 1, float>;
	overs[8][2] = Overlaps_C<8, 2, double, float>;
	degs[8][2] = Degrain_C<8, 2, float>;
	overs[8][4] = Overlaps_C<8, 4, double, float>;
	degs[8][4] = Degrain_C<8, 4, float>;
	overs[8][8] = Overlaps_C<8, 8, double, float>;
	degs[8][8] = Degrain_C<8, 8, float>;
	overs[8][16] = Overlaps_C<8, 16, double, float>;
	degs[8][16] = Degrain_C<8, 16, float>;
	overs[16][1] = Overlaps_C<16, 1, double, float>;
	degs[16][1] = Degrain_C<16, 1, float>;
	overs[16][2] = Overlaps_C<16, 2, double, float>;
	degs[16][2] = Degrain_C<16, 2, float>;
	overs[16][4] = Overlaps_C<16, 4, double, float>;
	degs[16][4] = Degrain_C<16, 4, float>;
	overs[16][8] = Overlaps_C<16, 8, double, float>;
	degs[16][8] = Degrain_C<16, 8, float>;
	overs[16][16] = Overlaps_C<16, 16, double, float>;
	degs[16][16] = Degrain_C<16, 16, float>;
	overs[16][32] = Overlaps_C<16, 32, double, float>;
	degs[16][32] = Degrain_C<16, 32, float>;
	overs[32][8] = Overlaps_C<32, 8, double, float>;
	degs[32][8] = Degrain_C<32, 8, float>;
	overs[32][16] = Overlaps_C<32, 16, double, float>;
	degs[32][16] = Degrain_C<32, 16, float>;
	overs[32][32] = Overlaps_C<32, 32, double, float>;
	degs[32][32] = Degrain_C<32, 32, float>;
	overs[32][64] = Overlaps_C<32, 64, double, float>;
	degs[32][64] = Degrain_C<32, 64, float>;
	overs[64][16] = Overlaps_C<64, 16, double, float>;
	degs[64][16] = Degrain_C<64, 16, float>;
	overs[64][32] = Overlaps_C<64, 32, double, float>;
	degs[64][32] = Degrain_C<64, 32, float>;
	overs[64][64] = Overlaps_C<64, 64, double, float>;
	degs[64][64] = Degrain_C<64, 64, float>;
	overs[64][128] = Overlaps_C<64, 128, double, float>;
	degs[64][128] = Degrain_C<64, 128, float>;
	overs[128][32] = Overlaps_C<128, 32, double, float>;
	degs[128][32] = Degrain_C<128, 32, float>;
	overs[128][64] = Overlaps_C<128, 64, double, float>;
	degs[128][64] = Degrain_C<128, 64, float>;
	overs[128][128] = Overlaps_C<128, 128, double, float>;
	degs[128][128] = Degrain_C<128, 128, float>;
	overs[128][256] = Overlaps_C<128, 256, double, float>;
	degs[128][256] = Degrain_C<128, 256, float>;
	overs[256][64] = Overlaps_C<256, 64, double, float>;
	degs[256][64] = Degrain_C<256, 64, float>;
	overs[256][128] = Overlaps_C<256, 128, double, float>;
	degs[256][128] = Degrain_C<256, 128, float>;
	overs[256][256] = Overlaps_C<256, 256, double, float>;
	degs[256][256] = Degrain_C<256, 256, float>;
	d->LimitChanges = LimitChanges_C<float>;
	d->ToPixels = ToPixels<double, float>;
	d->OVERS[0] = overs[nBlkSizeX][nBlkSizeY];
	d->DEGRAIN[0] = degs[nBlkSizeX][nBlkSizeY];
	d->OVERS[1] = d->OVERS[2] = overs[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
	d->DEGRAIN[1] = d->DEGRAIN[2] = degs[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
}

static void VS_CC mvdegrainCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	auto args = ArgumentList{ in };
	auto Core = VaporCore{ core };
	auto filter = "Degrain"s;

	MVDegrainData d;
	MVDegrainData* data;
	auto mvmulti = static_cast<Clip>(args["mvmulti"]);
	d.node = args["clip"];
	auto radius = mvmulti.FrameCount / d.node.FrameCount / 2;
	auto thsad = std::array<double, 3>{};
	auto thsad2 = thsad;

	d.radius = radius;
	d.vectors.resize(2 * radius);
	d.mvClips.resize(2 * radius);
	d.thSAD.resize(2 * radius);

	int err;
	const int m = vsapi->propNumElements(in, "thsad");
	const int n = vsapi->propNumElements(in, "limit");
	for (int i = 0; i < 3; ++i) {
		if (m == -1)
			thsad[0] = thsad[1] = thsad[2] = 400.;
		else
			if (i < m)
				thsad[i] = vsapi->propGetFloat(in, "thsad", i, nullptr);
			else
				thsad[i] = thsad[i - 1];
		if (n == -1)
			d.nLimit[0] = d.nLimit[1] = d.nLimit[2] = std::numeric_limits<double>::infinity();
		else
			if (i < n) {
				d.nLimit[i] = vsapi->propGetFloat(in, "limit", i, nullptr);
				if (d.nLimit[i] < 0.) {
					vsapi->setError(out, (filter + ": limit cannot be negative.").c_str());
					return;
				}
			}
			else
				d.nLimit[i] = d.nLimit[i - 1];
	}

	if (auto sad2Count = vsapi->propNumElements(in, "thsad2"); sad2Count == -1) {
		thsad2[0] = thsad[0];
		thsad2[1] = thsad[1];
		thsad2[2] = thsad[2];
	}
	else
		for (auto c : Range{ 3 })
			if (c < sad2Count)
				thsad2[c] = vsapi->propGetFloat(in, "thsad2", c, nullptr);
			else
				thsad2[c] = thsad2[c - 1];

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
	d.super = args["super"];
	char errorMsg[1024];
	const VSFrameRef* evil = vsapi->getFrame(0, d.super.VideoNode, errorMsg, 1024);
	if (!evil) {
		vsapi->setError(out, (filter + ": failed to retrieve first frame from super clip. Error message: " + errorMsg).c_str());

		return;
	}
	const VSMap* props = vsapi->getFramePropsRO(evil);
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
			vsapi->setError(out, (filter + ": required properties not found in first frame of super clip. Maybe clip didn't come from mvsf.Super? Was the first frame trimmed away?").c_str());
			return;
		}

	auto bvn = [&](auto n) {
		return Core["std"]["SelectEvery"]("clip", mvmulti, "cycle", radius * 2, "offsets", radius - n);
	};

	auto fvn = [&](auto n) {
		return Core["std"]["SelectEvery"]("clip", mvmulti, "cycle", radius * 2, "offsets", radius + n - 1);
	};

	for (auto r : Range{ radius }) {
		d.vectors[2 * r] = bvn(r + 1);
		d.vectors[2 * r + 1] = fvn(r + 1);
		for (auto c : Range{ 3 })
			d.thSAD[2 * r + 1][c] = d.thSAD[2 * r][c] = CosineAnnealing(thsad[c], thsad2[c], r + 1, radius);
	}

	for (int32_t r = 0; r < radius * 2; r++) {
		try {
			d.mvClips[r] = MVClipDicks{ d.vectors[r].VideoNode, d.nSCD1, d.nSCD2, vsapi };
		}
		catch (MVException& e) {
			vsapi->setError(out, (filter + ": " + e.what()).c_str());
			return;
		}
	}
	try {
		d.bleh = new MVFilter{ d.vectors[1].VideoNode, filter.c_str(), vsapi };
	}
	catch (MVException& e) {
		vsapi->setError(out, (filter + ": " + e.what()).c_str());
		return;
	}
	try {
		for (int32_t r = 0; r < radius * 2; r++)
			d.bleh->CheckSimilarity(&d.mvClips[r], "mvmulti");
	}
	catch (MVException& e) {
		vsapi->setError(out, (filter + ": " + e.what()).c_str());
		delete d.bleh;
		return;
	}

	for (auto& SADArray : d.thSAD) {
		SADArray[0] = SADArray[0] * d.mvClips[0].GetThSCD1() / d.nSCD1;
		SADArray[1] = SADArray[2] = SADArray[1] * d.mvClips[0].GetThSCD1() / d.nSCD1;
	}

	auto& supervi = d.super.ExposeVideoInfo();
	int32_t nSuperWidth = supervi.width;
	if (d.bleh->nHeight != nHeightS || d.bleh->nHeight != d.node.height || d.bleh->nWidth != nSuperWidth - d.nSuperHPad * 2 || d.bleh->nWidth != d.node.width) {
		vsapi->setError(out, (filter + ": wrong source or super clip frame size.").c_str());
		delete d.bleh;
		return;
	}
	if (!d.node.WithConstantFormat() || !d.node.WithConstantDimensions() || !d.node.IsSinglePrecision()) {
		vsapi->setError(out, (filter + ": input clip must be single precision fp, with constant dimensions.").c_str());
		delete d.bleh;
		return;
	}
	d.dstTempPitch = ((d.bleh->nWidth + 15) / 16) * 16 * 4 * 2;
	d.process[0] = d.node.colorFamily == cmRGB ? true : !!(d.YUVplanes & YPLANE);
	d.process[1] = d.node.colorFamily == cmRGB ? true : !!(d.YUVplanes & UPLANE & d.nSuperModeYUV);
	d.process[2] = d.node.colorFamily == cmRGB ? true : !!(d.YUVplanes & VPLANE & d.nSuperModeYUV);
	d.xSubUV = d.node.subSamplingW;
	d.ySubUV = d.node.subSamplingH;
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
		if (d.node.colorFamily != cmGray) {
			d.OverWins[1] = new OverlapWindows(d.nBlkSizeX[1], d.nBlkSizeY[1], d.nOverlapX[1], d.nOverlapY[1]);
			d.OverWins[2] = d.OverWins[1];
		}
	}
	selectFunctions(&d);
	data = new MVDegrainData;
	*data = d;
	vsapi->createFilter(in, out, filter.c_str(), mvdegrainInit, mvdegrainGetFrame, mvdegrainFree, fmParallel, 0, data, core);
}

void mvdegrainRegister(VSRegisterFunction registerFunc, VSPlugin* plugin) {
	registerFunc("Degrain",
		"clip:clip;"
		"super:clip;"
		"mvmulti:clip;"
		"thsad:float[]:opt;"
		"thsad2:float[]:opt;"
		"plane:int:opt;"
		"limit:float[]:opt;"
		"thscd1:float:opt;"
		"thscd2:float:opt;"
		, mvdegrainCreate, 0, plugin);
}