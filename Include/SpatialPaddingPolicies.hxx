#pragma once
#include "Infrastructure.hxx"

namespace PaddingPolicies {
	auto ReflectCoordinate(auto x, auto Bound) {
		x = std::abs(x);
		x -= Bound - 1;
		x = -std::abs(x);
		x += Bound - 1;
		return x;
	}
}

namespace PaddingPolicies::Spatial {
	auto Zero = [](auto Canvas, auto Width, auto Height, auto y, auto x) {
		using PixelType = std::decay_t<decltype(Canvas[0][0])>;
		return static_cast<PixelType>(0);
	};

	auto Repeat = [](auto Canvas, auto Width, auto Height, auto y, auto x) {
		x = Min(Max(x, 0), Width - 1);
		y = Min(Max(y, 0), Height - 1);
		return Canvas[y][x];
	};

	auto Reflect = [](auto Canvas, auto Width, auto Height, auto y, auto x) {
		x = ReflectCoordinate(x, Width);
		y = ReflectCoordinate(y, Height);
		return Canvas[y][x];
	};

	auto Default = Repeat;
}