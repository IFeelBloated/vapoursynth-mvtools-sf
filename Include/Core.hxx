#pragma once
#include "Plugin.hxx"

struct VaporCore final {
	self(Instance, static_cast<VSCore*>(nullptr));
	auto FetchFormat(auto FormatID) {
		return VaporGlobals::API->getFormatPreset(FormatID, Instance);
	}
	auto AllocateFrame(auto Format, auto Width, auto Height) {
		return VaporGlobals::API->newVideoFrame(Format, Width, Height, nullptr, Instance);
	}
	auto CopyFrameProperties(auto&& Source, auto&& Destination) {
		VaporGlobals::API->copyFrameProps(Source, Destination, Instance);
	}
	auto CreateNewFrameFrom(auto&& ReferenceFrame) {
		using PixelType = std::decay_t<decltype(ReferenceFrame[0][0][0])>;
		auto Format = ReferenceFrame.Format;
		auto Width = ReferenceFrame[0].Width;
		auto Height = ReferenceFrame[0].Height;
		auto Properties = ReferenceFrame.RawFrame;
		auto AllocatedFrame = VaporGlobals::API->newVideoFrame(Format, Width, Height, Properties, Instance);
		return Frame<PixelType>{ AllocatedFrame };
	}
	auto operator[](auto&& Namespace) {
		return VaporPlugin{ .Plugin = VaporGlobals::API->getPluginByNs(ExposeCString(Namespace), Instance) };
	}
	operator auto() {
		return Instance;
	}
};