#pragma once
#include "VapourSynth.h"
#include "Infrastructure.hxx"

struct MaterializedFormat : VSFormat {
	auto IsSinglePrecision() {
		return SampleType == VSSampleType::stFloat && BitsPerSample == 32;
	}
	auto IsRGB() {
		return ColorFamily == VSColorFamily::cmRGB;
	}
	auto IsGray() {
		return ColorFamily == VSColorFamily::cmGray;
	}
	auto IsYUV() {
		return ColorFamily == VSColorFamily::cmYUV;
	}
	auto Is444() {
		return HorizontalSubsamplingFactor == 0 && VerticalSubsamplingFactor == 0;
	}
};