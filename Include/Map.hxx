#pragma once
#include "Clip.hxx"

struct ReadonlyItem final {
	self(Map, static_cast<const VSMap*>(nullptr));
	self(Key, ""s);
	self(Index, 0_ptrdiff);
	struct Iterator final {
		self(State, static_cast<ReadonlyItem*>(nullptr));
		self(Index, 0_ptrdiff);
		auto& operator*() const {
			State->Index = Index;
			return *State;
		}
		auto& operator++() {
			++Index;
			return *this;
		}
		auto operator!=(auto&& OtherIterator) const {
			return Index != OtherIterator.Index;
		}
	};
	auto Size() {
		return Max(VaporGlobals::API->propNumElements(Map, Key.data()), 0);
	}
	auto Exists() {
		return Index < Size();
	}
	auto Type() {
		return VaporGlobals::API->propGetType(Map, Key.data());
	}
	auto Begin() {
		return Iterator{ .State = this, .Index = 0 };
	}
	auto End() {
		return Iterator{ .State = this, .Index = Size() };
	}
	auto& operator[](auto Index) {
		this->Index = Index;
		return *this;
	}
	operator Clip() {
		return Clip{ VaporGlobals::API->propGetNode(Map, Key.data(), Index, nullptr) };
	}
	operator double() {
		return VaporGlobals::API->propGetFloat(Map, Key.data(), Index, nullptr);
	}
	operator float() {
		return static_cast<float>(VaporGlobals::API->propGetFloat(Map, Key.data(), Index, nullptr));
	}
	operator std::int64_t() {
		return VaporGlobals::API->propGetInt(Map, Key.data(), Index, nullptr);
	}
	operator int() {
		return static_cast<int>(VaporGlobals::API->propGetInt(Map, Key.data(), Index, nullptr));
	}
	operator bool() {
		return !!VaporGlobals::API->propGetInt(Map, Key.data(), Index, nullptr);
	}
	operator std::string() {
		return std::string{ VaporGlobals::API->propGetData(Map, Key.data(), Index, nullptr) };
	}
};

struct WritableItem final {
	self(Map, static_cast<VSMap*>(nullptr));
	self(Key, ""s);
	auto Set(auto&& Value, auto AppendMode) {
		if constexpr (isinstance(Value, double) || isinstance(Value, float))
			return VaporGlobals::API->propSetFloat(Map, Key.data(), Value, AppendMode) == 0;
		else if constexpr (isinstance(Value, std::int64_t) || isinstance(Value, int) || isinstance(Value, bool))
			return VaporGlobals::API->propSetInt(Map, Key.data(), Value, AppendMode) == 0;
		else if constexpr (isinstance(Value, char*) || isinstance(Value, const char*))
			return VaporGlobals::API->propSetData(Map, Key.data(), Value, -1, AppendMode) == 0;
		else if constexpr (isinstance(Value, std::string) || isinstance(Value, std::string_view))
			return VaporGlobals::API->propSetData(Map, Key.data(), Value.data(), Value.size(), AppendMode) == 0;
		else if constexpr (isinstance(Value, Clip))
			return VaporGlobals::API->propSetNode(Map, Key.data(), Value.VideoNode, AppendMode) == 0;
		else
			return false;
	}
	auto Erase() {
		return VaporGlobals::API->propDeleteKey(Map, Key.data()) == 1;
	}
	auto& operator=(auto&& Value) {
		if constexpr (isinstance(Value, WritableItem)) {
			if (this != &Value) {
				Map = Value.Map;
				if constexpr (std::is_rvalue_reference_v<decltype(Value)>)
					Key = std::move(Value.Key);
				else
					Key = Value.Key;
			}
		}
		else
			Set(Forward(Value), VSPropAppendMode::paReplace);
		return *this;
	}
	auto& operator+=(auto&& Value) {
		Set(Forward(Value), VSPropAppendMode::paAppend);
		return *this;
	}
	auto& operator|=(auto&& Value) {
		Set(Forward(Value), VSPropAppendMode::paTouch);
		return *this;
	}
};

struct ArgumentList final {
	self(InputMap, static_cast<const VSMap*>(nullptr));
	auto operator[](auto&& Parameter) {
		return ReadonlyItem{ .Map = InputMap, .Key = Forward(Parameter) };
	}
};

template<typename FilterType>
struct Controller final {
	self(OutputMap, static_cast<VSMap*>(nullptr));
	auto RaiseError(auto&& ErrorMessage) {
		auto Caption = FilterType::Name + ": "s;
		auto DecoratedMessage = Caption + ErrorMessage;
		VaporGlobals::API->setError(OutputMap, ExposeCString(DecoratedMessage));
		return false;
	}
	auto Receive(auto&& VideoClip) {
		return VaporGlobals::API->propSetNode(OutputMap, "clip", VideoClip.VideoNode, VSPropAppendMode::paAppend) == 0;
	}
	operator auto() {
		return OutputMap;
	}
};

template<typename PixelType>
auto Frame<PixelType>::GetProperty(auto&& Key) {
	if constexpr (std::is_const_v<PixelType>)
		return ReadonlyItem{ .Map = PropertyMap, .Key = Forward(Key) };
	else
		return WritableItem{ .Map = PropertyMap, .Key = Forward(Key) };
}