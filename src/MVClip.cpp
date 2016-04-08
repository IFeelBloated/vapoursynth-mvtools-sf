#include "MVClip.h"

MVClipDicks::MVClipDicks(VSNodeRef *vectors, double _nSCD1, double _nSCD2, const VSAPI *_vsapi) :
	vsapi(_vsapi) {
	char errorMsg[1024];
	const VSFrameRef *evil = vsapi->getFrame(0, vectors, errorMsg, 1024);
	if (!evil)
		throw MVException(std::string("Failed to retrieve first frame from some motion clip. Error message: ").append(errorMsg).c_str());
	const MVAnalysisData *pAnalyzeFilter = reinterpret_cast<const MVAnalysisData *>(vsapi->getReadPtr(evil, 0) + sizeof(int32_t));
	if (pAnalyzeFilter->GetMagicKey() != MOTION_MAGIC_KEY) {
		vsapi->freeFrame(evil);
		throw MVException("Invalid motion vector clip.");
	}
	nBlkSizeX = pAnalyzeFilter->GetBlkSizeX();
	nBlkSizeY = pAnalyzeFilter->GetBlkSizeY();
	nPel = pAnalyzeFilter->GetPel();
	isBackward = pAnalyzeFilter->IsBackward();
	nLvCount = pAnalyzeFilter->GetLevelCount();
	nDeltaFrame = pAnalyzeFilter->GetDeltaFrame();
	nWidth = pAnalyzeFilter->GetWidth();
	nHeight = pAnalyzeFilter->GetHeight();
	nMagicKey = pAnalyzeFilter->GetMagicKey();
	nOverlapX = pAnalyzeFilter->GetOverlapX();
	nOverlapY = pAnalyzeFilter->GetOverlapY();
	xRatioUV = pAnalyzeFilter->GetXRatioUV();
	yRatioUV = pAnalyzeFilter->GetYRatioUV();
	nVPadding = pAnalyzeFilter->GetVPadding();
	nHPadding = pAnalyzeFilter->GetHPadding();
	nMotionFlags = pAnalyzeFilter->GetMotionFlags();
	nBlkX = pAnalyzeFilter->GetBlkX();
	nBlkY = pAnalyzeFilter->GetBlkY();
	nBlkCount = nBlkX * nBlkY;
	double maxSAD = 8. * 8. * 255.;
	if (_nSCD1 > maxSAD)
		throw MVException(std::string("thscd1 can be at most ").append(std::to_string(maxSAD)).append("."));
	int32_t referenceBlockSize = 8 * 8;
	nSCD1 = _nSCD1 * (nBlkSizeX * nBlkSizeY) / referenceBlockSize;
	if (pAnalyzeFilter->IsChromaMotion())
		nSCD1 += nSCD1 / (xRatioUV * yRatioUV) * 2;
	nSCD1 = nSCD1 / 255.;
	nSCD2 = _nSCD2 * nBlkCount / 256.;
	vsapi->freeFrame(evil);
}

MVClipDicks::~MVClipDicks() {}

MVClipBalls::MVClipBalls(MVClipDicks *_dicks, const VSAPI *_vsapi) :
	dicks(_dicks),
	vsapi(_vsapi) {
	FakeGroupOfPlanes::Create(dicks->GetBlkSizeX(), dicks->GetBlkSizeY(), dicks->GetLevelCount(), dicks->GetPel(), dicks->GetOverlapX(), dicks->GetOverlapY(), dicks->GetYRatioUV(), dicks->GetBlkX(), dicks->GetBlkY());
}

MVClipBalls::~MVClipBalls() {}

void MVClipBalls::Update(const VSFrameRef *fn) {
	const int32_t *pMv = reinterpret_cast<const int32_t*>(vsapi->getReadPtr(fn, 0));
	int32_t _headerSize = pMv[0];
	int32_t nMagicKey1 = pMv[1];
	if (nMagicKey1 != MOTION_MAGIC_KEY)
		throw MVException("MVTools: invalid motion vector clip. Who knows where this error came from exactly?");
	int32_t nVersion1 = pMv[2];
	if (nVersion1 != MVANALYSIS_DATA_VERSION)
		throw MVException("MVTools: incompatible version of motion vector clip. Who knows where this error came from exactly?");
	pMv += _headerSize / sizeof(int32_t);
	FakeGroupOfPlanes::Update(pMv);
}

bool  MVClipBalls::IsUsable() const {
	return (!FakeGroupOfPlanes::IsSceneChange(dicks->GetThSCD1(), dicks->GetThSCD2())) && FakeGroupOfPlanes::IsValid();
}
