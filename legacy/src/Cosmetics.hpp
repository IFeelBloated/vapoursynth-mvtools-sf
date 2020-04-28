#pragma once
#include <type_traits>
#include <cstdint>

#define self(ClassMember, Initialization) std::decay_t<decltype(Initialization)> ClassMember = Initialization

constexpr auto operator""_i32(unsigned long long value) {
	return static_cast<std::int32_t>(value);
}

constexpr auto operator""_i64(unsigned long long value) {
	return static_cast<std::int64_t>(value);
}

static auto CastToConstantPointer = [](const auto *Pointer) {
	return Pointer;
};