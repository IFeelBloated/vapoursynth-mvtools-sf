// Author: Manao
// Copyright(c)2006 A.G.Balakhnin aka Fizick - overlap, YUY2, sharp
// See legal notice in Copying.txt for more information
//
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

#include "MVClip.h"

MVClipDicks::MVClipDicks(VSNodeRef *vectors, float _nSCD1, float _nSCD2, const VSAPI *_vsapi) :
    vsapi(_vsapi)
{
    char errorMsg[1024];
    const VSFrameRef *evil = vsapi->getFrame(0, vectors, errorMsg, 1024);
    if (!evil)
        throw MVException(std::string("Failed to retrieve first frame from some motion clip. Error message: ").append(errorMsg).c_str());

    // XXX This really should be passed as a frame property.
    const MVAnalysisData *pAnalyzeFilter = reinterpret_cast<const MVAnalysisData *>(vsapi->getReadPtr(evil, 0) + sizeof(int));

    // 'magic' key, just to check :
    if ( pAnalyzeFilter->GetMagicKey() != MOTION_MAGIC_KEY ) {
        vsapi->freeFrame(evil);
        throw MVException("Invalid motion vector clip.");
    }

    // MVAnalysisData
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


    float maxSAD = 8.f * 8.f * 255.f;

    if (_nSCD1 > maxSAD)
        throw MVException(std::string("thscd1 can be at most ").append(std::to_string(maxSAD)).append("."));

    // SCD thresholds
    int referenceBlockSize = 8 * 8;
    nSCD1 = _nSCD1 * (nBlkSizeX * nBlkSizeY) / referenceBlockSize;
    if ( pAnalyzeFilter->IsChromaMotion() )
        nSCD1 += nSCD1 / (xRatioUV * yRatioUV) * 2;

    float pixelMax = 1.f;
    nSCD1 = float((double)nSCD1 * pixelMax / 255.0);

    nSCD2 = _nSCD2 * nBlkCount / 256;

    vsapi->freeFrame(evil);
}

MVClipDicks::~MVClipDicks()
{
}


MVClipBalls::MVClipBalls(MVClipDicks *_dicks, const VSAPI *_vsapi) :
    dicks(_dicks),
    vsapi(_vsapi)
{
    FakeGroupOfPlanes::Create(dicks->GetBlkSizeX(), dicks->GetBlkSizeY(), dicks->GetLevelCount(), dicks->GetPel(), dicks->GetOverlapX(), dicks->GetOverlapY(), dicks->GetYRatioUV(), dicks->GetBlkX(), dicks->GetBlkY());
}

MVClipBalls::~MVClipBalls()
{
}


void MVClipBalls::Update(const VSFrameRef *fn)

{
    const int *pMv = reinterpret_cast<const int*>(vsapi->getReadPtr(fn, 0));
    int _headerSize = pMv[0];
    int nMagicKey1 = pMv[1];

    if (nMagicKey1 != MOTION_MAGIC_KEY)
        throw MVException("MVTools: invalid motion vector clip. Who knows where this error came from exactly?");

    int nVersion1 = pMv[2];

    if (nVersion1 != MVANALYSIS_DATA_VERSION)
        throw MVException("MVTools: incompatible version of motion vector clip. Who knows where this error came from exactly?");

    pMv += _headerSize/sizeof(int); // go to data - v1.8.1

    FakeGroupOfPlanes::Update(pMv);// fixed a bug with lost frames
}


bool  MVClipBalls::IsUsable() const
{
    return (!FakeGroupOfPlanes::IsSceneChange(dicks->GetThSCD1(), dicks->GetThSCD2())) && FakeGroupOfPlanes::IsValid();
}

