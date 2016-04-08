#ifndef MVTOOLS_FAKEGROUPOFPLANES_H
#define MVTOOLS_FAKEGROUPOFPLANES_H
#include "FakePlaneOfBlocks.h"

class FakeGroupOfPlanes {
	int32_t nLvCount_;
	bool validity;
	int32_t nWidth_B;
	int32_t nHeight_B;
	int32_t yRatioUV_B;
	FakePlaneOfBlocks **planes;
	static inline bool GetValidity(const int32_t *array) { return (array[1] == 1); }
public:
	FakeGroupOfPlanes();
	~FakeGroupOfPlanes();
	void Create(int32_t _nBlkSizeX, int32_t _nBlkSizeY, int32_t _nLevelCount, int32_t _nPel, int32_t _nOverlapX, int32_t _nOverlapY, int32_t _yRatioUV, int32_t _nBlkX, int32_t _nBlkY);
	void Update(const int32_t *array);
	bool IsSceneChange(double nThSCD1, double nThSCD2) const;
	inline const FakePlaneOfBlocks& operator[](const int32_t i) const {
		return *(planes[i]);
	}
	inline bool IsValid() const { return validity; }
	inline int32_t GetPitch() const { return nWidth_B; }
	inline int32_t GetPitchUV() const { return nWidth_B / 2; }
	inline const FakePlaneOfBlocks& GetPlane(int32_t i) const { return *(planes[i]); }
};

#endif

