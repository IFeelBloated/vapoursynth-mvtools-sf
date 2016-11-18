#ifndef MVTOOLS_FAKEBLOCKDATA_H
#define MVTOOLS_FAKEBLOCKDATA_H
#include "MVInterface.h"

class FakeBlockData {
	int32_t x;
	int32_t y;
	VECTOR vector;
public:
	FakeBlockData();
	FakeBlockData(int32_t _x, int32_t _y);
	~FakeBlockData();
	void Init(int32_t _x, int32_t _y);
	void Update(const int32_t *array);
	inline int32_t GetX() const { return x; }
	inline int32_t GetY() const { return y; }
	inline VECTOR GetMV() const { return vector; }
	inline int32_t GetSAD() const { return vector.sad; }
};

#endif
