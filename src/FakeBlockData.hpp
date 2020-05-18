#pragma once
#include "MVInterface.h"
#include "Interface.vxx"

class FakeBlockData final {
	self(x, 0_i32);
	self(y, 0_i32);
	self(Vector, VectorStructure{});
public:
	FakeBlockData() = default;
	FakeBlockData(std::int32_t x, std::int32_t y) {
		this->x = x;
		this->y = y;
	}
	FakeBlockData(FakeBlockData &&) = default;
	FakeBlockData(const FakeBlockData &) = default;
	auto operator=(FakeBlockData &&)->decltype(*this) = default;
	auto operator=(const FakeBlockData &)->decltype(*this) = default;
	~FakeBlockData() = default;
	auto Update(const VectorStructure *NewVectorPointer) {
		Vector = *NewVectorPointer;
	}
	auto GetX() const { 
		return x; 
	}
	auto GetY() const { 
		return y; 
	}
	auto GetMV() const { 
		return Vector; 
	}
	auto GetSAD() const {
		return Vector.sad;
	}
};