#pragma once
#include <cstdint>

struct DCTClass {
	int sizex = 0;
	int sizey = 0;
	int dctmode = 0;
	DCTClass() = default;
	virtual ~DCTClass() = default;
	virtual auto DCTBytes2D(const uint8_t *, int, uint8_t *, int)->void = 0;
};
