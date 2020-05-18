#pragma once
#include <cstdint>
#include "Interface.vxx"

struct DCTClass {
	self(sizex, 0);
	self(sizey, 0);
	self(dctmode, 0);
	DCTClass() = default;
	DCTClass(DCTClass &&) = default;
	DCTClass(const DCTClass &) = default;
	auto operator=(DCTClass &&)->decltype(*this) = default;
	auto operator=(const DCTClass &)->decltype(*this) = default;
	virtual ~DCTClass() = default;
	virtual auto DCTBytes2D(const std::uint8_t *, int, std::uint8_t *, int)->void = 0;
};
