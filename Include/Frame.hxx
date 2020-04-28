#pragma once
#include "Globals.hxx"
#include "Plane.hxx"
#include "Format.hxx"
#include "SpatialPaddingPolicies.hxx"

template<typename PixelType>
struct Frame final : MaterializedFormat {
	self(RawFrame, static_cast<VSFrameRef*>(nullptr));
	self(Planes, std::array{ Plane<PixelType>{}, Plane<PixelType>{}, Plane<PixelType>{} });
	self(Format, static_cast<const VSFormat*>(nullptr));
	self(PropertyMap, static_cast<VSMap*>(nullptr));
	Frame() = default;
	Frame(auto RawFrame) {
		auto ConstructPlane = [&](auto Index) {
			auto Width = VaporGlobals::API->getFrameWidth(RawFrame, Index);
			auto Height = VaporGlobals::API->getFrameHeight(RawFrame, Index);
			auto Stride = VaporGlobals::API->getStride(RawFrame, Index) / sizeof(PixelType);
			auto GetPlanePointer = [&]() {
				if constexpr (std::is_const_v<PixelType>)
					return VaporGlobals::API->getReadPtr(RawFrame, Index);
				else
					return VaporGlobals::API->getWritePtr(RawFrame, Index);
			};
			return Plane<PixelType>{ GetPlanePointer(), Width, Height, Stride, PaddingPolicies::Spatial::Default };
		};
		this->RawFrame = RawFrame;
		this->Format = VaporGlobals::API->getFrameFormat(RawFrame);
		SynchronizeFormat();
		for (auto Index : Range{ PlaneCount })
			Planes[Index] = ConstructPlane(Index);
		if constexpr (std::is_const_v<PixelType>)
			PropertyMap = PointerRemoveConstant(VaporGlobals::API->getFramePropsRO(RawFrame));
		else
			PropertyMap = VaporGlobals::API->getFramePropsRW(RawFrame);
	}
	Frame(const Frame& OtherFrame) {
		*this = OtherFrame;
	}
	Frame(Frame&& OtherFrame) {
		*this = std::move(OtherFrame);
	}
	auto& operator=(const Frame& OtherFrame) {
		if (this != &OtherFrame) {
			this->~Frame();
			RawFrame = PointerRemoveConstant(VaporGlobals::API->cloneFrameRef(OtherFrame.RawFrame));
			Planes = OtherFrame.Planes;
			Format = OtherFrame.Format;
			PropertyMap = OtherFrame.PropertyMap;
			SynchronizeFormat();
		}
		return *this;
	}
	auto& operator=(Frame&& OtherFrame) {
		if (this != &OtherFrame) {
			std::swap(RawFrame, OtherFrame.RawFrame);
			Planes = std::move(OtherFrame.Planes);
			Format = OtherFrame.Format;
			PropertyMap = OtherFrame.PropertyMap;
			SynchronizeFormat();
		}
		return *this;
	}
	~Frame() {
		VaporGlobals::API->freeFrame(RawFrame);
	}
	auto SynchronizeFormat() {
		auto& EnclosedFormat = static_cast<VSFormat&>(*this);
		EnclosedFormat = *Format;
	}
	auto GetProperty(auto&&);
	decltype(auto) operator[](auto&& x) {
		if constexpr (isinstance(x, const char*) || isinstance(x, char*) || isinstance(x, std::string) || isinstance(x, std::string_view))
			return GetProperty(Forward(x));
		else
			return Planes[x];
	}
	auto Leak() {
		auto LeakedFrame = RawFrame;
		RawFrame = nullptr;
		return PointerAddConstant(LeakedFrame);
	}
	operator auto() {
		if constexpr (std::is_const_v<PixelType>)
			return PointerAddConstant(RawFrame);
		else
			return RawFrame;
	}
};

template<typename FilterType>
struct VaporFrameContext final {
	self(Context, static_cast<VSFrameContext*>(nullptr));
	auto RaiseError(auto&& ErrorMessage) {
		auto NullFrame = static_cast<const VSFrameRef*>(nullptr);
		auto Caption = FilterType::Name + ": "s;
		auto DecoratedMessage = Caption + ErrorMessage;
		VaporGlobals::API->setFilterError(ExposeCString(DecoratedMessage), Context);
		return NullFrame;
	}
	operator auto() {
		return Context;
	}
};