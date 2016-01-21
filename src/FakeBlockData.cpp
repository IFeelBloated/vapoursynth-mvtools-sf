#include "FakeBlockData.h"

void FakeBlockData::Init(int _x, int _y) {
	x = _x;
	y = _y;
}

FakeBlockData::FakeBlockData(int _x, int _y) {
	x = _x;
	y = _y;
}

FakeBlockData::FakeBlockData() {
	x = 0;
	y = 0;
}

FakeBlockData::~FakeBlockData() {}

void FakeBlockData::Update(const int *array) {
	vector.x = array[0];
	vector.y = array[1];
	vector.sad = array[2];
}
