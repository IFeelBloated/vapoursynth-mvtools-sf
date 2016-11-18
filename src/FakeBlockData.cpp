#include "FakeBlockData.h"

void FakeBlockData::Init(int32_t _x, int32_t _y) {
	x = _x;
	y = _y;
}

FakeBlockData::FakeBlockData(int32_t _x, int32_t _y) {
	x = _x;
	y = _y;
}

FakeBlockData::FakeBlockData() {
	x = 0;
	y = 0;
}

FakeBlockData::~FakeBlockData() {}

void FakeBlockData::Update(const int32_t *array) {
	vector.x = array[0];
	vector.y = array[1];
	vector.sad = array[2];
}
