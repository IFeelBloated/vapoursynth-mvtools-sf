#pragma once
#include "Frame.hxx"

template<typename PixelType>
struct Buffer final {
	self(Width, 0_size);
	self(Height, 0_size);
	self(Canvas, std::vector<PixelType*>{});
	Buffer() = default;
	Buffer(auto Width, auto Height) {
		this->Width = Width;
		this->Height = Height;
		auto Stride = DetermineStride();
		auto Origin = reinterpret_cast<PixelType*>(std::aligned_alloc(32, Stride * Height * sizeof(PixelType)));
		Canvas.resize(Height);
		for (auto y : Range{ Height })
			Canvas[y] = Origin + y * Stride;
	}
	Buffer(const Buffer& OtherBuffer) {
		*this = OtherBuffer;
	}
	Buffer(Buffer&& OtherBuffer) {
		*this = std::move(OtherBuffer);
	}
	auto& operator=(const Buffer& OtherBuffer) {
		if (this != &OtherBuffer) {
			if (Width != OtherBuffer.Width || Height != OtherBuffer.Height) {
				this->~Buffer();
				new(this) Buffer{ OtherBuffer.Width, OtherBuffer.Height };
			}
			std::copy(OtherBuffer.Canvas[0], OtherBuffer.Canvas[0] + DetermineStride() * Height, Canvas[0]);
		}
		return *this;
	}
	auto& operator=(Buffer&& OtherBuffer) {
		if (this != &OtherBuffer) {
			std::swap(Canvas, OtherBuffer.Canvas);
			Width = OtherBuffer.Width;
			Height = OtherBuffer.Height;
		}
		return *this;
	}
	~Buffer() {
		if (Canvas.size() != 0)
			std::free(Canvas[0]);
	}
	auto operator[](auto y) {
		return Canvas[y];
	}
	auto DetermineStride() {
		auto RowSize = Width * sizeof(PixelType);
		if (RowSize % 32 != 0)
			RowSize = (RowSize / 32 + 1) * 32;
		return RowSize / sizeof(PixelType);
	}
	auto AccessAsPlane() {
		return Plane<const PixelType>{ Canvas[0], Width, Height, DetermineStride(), PaddingPolicies::Spatial::Default };
	}
};