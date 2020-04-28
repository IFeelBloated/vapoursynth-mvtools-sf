#pragma once
#include "Frame.hxx"
#include "TemporalPaddingPolicies.hxx"

struct Clip final : VSVideoInfo, MaterializedFormat {
	self(VideoNode, static_cast<VSNodeRef*>(nullptr));
	template<typename ContainerType>
	struct Sequence final {
		self(Container, ContainerType{});
		self(Radius, 0_ptrdiff);
		struct Offset final {
			self(State, static_cast<Sequence*>(nullptr));
			self(tOffset, 0_ptrdiff);
			auto& operator[](auto t) {
				auto& ReferenceSequence = *State;
				return ReferenceSequence[t + tOffset];
			}
			auto GetCoordinate() {
				return tOffset;
			}
			auto View(auto t) {
				return Offset{ .State = State, .tOffset = tOffset + t };
			}
		};
		auto& operator[](auto t) {
			return Container[t + Radius];
		}
		auto View(auto t) {
			return Offset{ .State = this, .tOffset = t };
		}
	};
	auto& ExposeVideoInfo() {
		return static_cast<VSVideoInfo&>(*this);
	}
	auto& ExposeVideoInfo() const {
		return static_cast<const VSVideoInfo&>(*this);
	}
	auto SynchronizeFormat() {
		if (auto& EnclosedFormat = static_cast<VSFormat&>(*this); Format != nullptr)
			EnclosedFormat = *Format;
	}
	Clip() = default;
	Clip(auto RawClip) {
		auto& VideoInfo = ExposeVideoInfo();
		VideoNode = RawClip;
		VideoInfo = *VaporGlobals::API->getVideoInfo(RawClip);
		SynchronizeFormat();
	}
	auto& operator=(const Clip& OtherClip) {
		if (auto& VideoInfo = ExposeVideoInfo(); this != &OtherClip) {
			this->~Clip();
			VideoNode = VaporGlobals::API->cloneNodeRef(OtherClip.VideoNode);
			VideoInfo = OtherClip.ExposeVideoInfo();
			SynchronizeFormat();
		}
		return *this;
	}
	auto& operator=(Clip&& OtherClip) {
		if (auto& VideoInfo = ExposeVideoInfo(); this != &OtherClip) {
			std::swap(VideoNode, OtherClip.VideoNode);
			VideoInfo = std::move(OtherClip.ExposeVideoInfo());
			SynchronizeFormat();
		}
		return *this;
	}
	Clip(const Clip& OtherClip) {
		*this = OtherClip;
	}
	Clip(Clip&& OtherClip) {
		*this = std::move(OtherClip);
	}
	~Clip() {
		VaporGlobals::API->freeNode(VideoNode);
	}
	auto RequestFrame(auto Index, auto FrameContext) {
		if (Index >= 0 && Index < FrameCount)
			VaporGlobals::API->requestFrameFilter(Index, VideoNode, FrameContext);
	}
	auto RequestFrames(auto Index, auto Radius, auto FrameContext) {
		for (auto Offset : Range{ Index - Radius, Index + Radius + 1 })
			RequestFrame(Offset, FrameContext);
	}
	template<typename PixelType>
	auto GetFrame(auto Index, auto&& PaddingPolicy, auto FrameContext, auto&& ...AuxiliaryArguments) {
		auto WrapAsFrame = [](auto RawFrame) {
			return Frame<PixelType>{ PointerRemoveConstant(RawFrame) };
		};
		if (Index < 0 || Index >= FrameCount)
			return WrapAsFrame(PaddingPolicy(*this, Index, FrameContext, Forward(AuxiliaryArguments)...));
		else
			return WrapAsFrame(VaporGlobals::API->getFrameFilter(Index, VideoNode, FrameContext));
	}
	template<typename PixelType>
	auto GetFrame(auto Index, auto FrameContext) {
		return GetFrame<PixelType>(Index, PaddingPolicies::Temporal::Default, FrameContext);
	}
	template<typename PixelType>
	auto GetFrames(auto Index, auto Radius, auto&& PaddingPolicy, auto FrameContext, auto&& ...AuxiliaryArguments) {
		using ContainerType = std::vector<Frame<PixelType>>;
		auto Container = ContainerType{};
		Container.reserve(2 * Radius + 1);
		for (auto Offset : Range{ Index - Radius, Index + Radius + 1 })
			Container.push_back(GetFrame<PixelType>(Offset, PaddingPolicy, FrameContext, Forward(AuxiliaryArguments)...));
		return Sequence<ContainerType>{ .Container = std::move(Container), .Radius = Radius };
	}
	template<typename PixelType>
	auto GetFrames(auto Index, auto Radius, auto FrameContext) {
		return GetFrames<PixelType>(Index, Radius, PaddingPolicies::Temporal::Default, FrameContext);
	}
	auto& PeekFrameFormat(auto Index, auto FrameContext) {
		auto RawFrame = VaporGlobals::API->getFrameFilter(Index, VideoNode, FrameContext);
		auto Format = VaporGlobals::API->getFrameFormat(RawFrame);
		VaporGlobals::API->freeFrame(RawFrame);
		return static_cast<MaterializedFormat&>(*Format);
	}
	auto WithConstantDimensions() {
		return Width != 0 && Height != 0;
	}
	auto WithConstantFormat() {
		return Format != nullptr;
	}
	auto ContainsVideoReference() {
		return VideoNode != nullptr;
	}
};