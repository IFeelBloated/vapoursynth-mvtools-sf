#ifndef MVTOOLS_MVCLIP_H
#define MVTOOLS_MVCLIP_H
#include "FakeGroupOfPlanes.h"

class MVClipDicks : public MVAnalysisData {
	int nBlkCount;
	float nSCD1;
	float nSCD2;
	const VSAPI *vsapi;
public:
	MVClipDicks(VSNodeRef *vectors, float nSCD1, float nSCD2, const VSAPI *_vsapi);
	~MVClipDicks();
	inline int GetBlkCount() const { return nBlkCount; }
	inline float GetThSCD1() const { return nSCD1; }
	inline float GetThSCD2() const { return nSCD2; }
};

class MVClipBalls : public FakeGroupOfPlanes {
	MVClipDicks *dicks;
	const VSAPI *vsapi;
public:
	MVClipBalls(MVClipDicks *_dicks, const VSAPI *_vsapi);
	~MVClipBalls();
	void Update(const VSFrameRef *fn);
	inline const FakeBlockData& GetBlock(int nLevel, int nBlk) const { return GetPlane(nLevel)[nBlk]; }
	bool IsUsable() const;
	bool IsSceneChange(float nSCD1, float nSCD2) const { return FakeGroupOfPlanes::IsSceneChange(nSCD1, nSCD2); }
};

#endif

