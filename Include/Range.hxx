#pragma once
#include "Infrastructure.hxx"

class Range final {
	struct Iterator final {
		self(Cursor, 0_ptrdiff);
		self(Step, 0_ptrdiff);
		auto operator*() const {
			return Cursor;
		}
		auto& operator++() {
			Cursor += Step;
			return *this;
		}
		auto operator!=(auto&& OtherIterator) const {
			if (Step > 0)
				return Cursor < OtherIterator.Cursor;
			else
				return Cursor > OtherIterator.Cursor;
		}
	};
	self(Startpoint, 0_ptrdiff);
	self(Endpoint, 0_ptrdiff);
	self(Step, 1_ptrdiff);
public:
	Range() = default;
	Range(auto Endpoint) {
		if (Endpoint < 0)
			Step = -1;
		this->Endpoint = static_cast<std::ptrdiff_t>(Endpoint);
	}
	Range(auto Startpoint, auto Endpoint) {
		if (Startpoint > Endpoint)
			Step = -1;
		this->Startpoint = static_cast<std::ptrdiff_t>(Startpoint);
		this->Endpoint = static_cast<std::ptrdiff_t>(Endpoint);
	}
	Range(auto Startpoint, auto Endpoint, auto Step) {
		this->Startpoint = static_cast<std::ptrdiff_t>(Startpoint);
		this->Endpoint = static_cast<std::ptrdiff_t>(Endpoint);
		this->Step = static_cast<std::ptrdiff_t>(Step);
	}
	auto Begin() const {
		return Iterator{ .Cursor = Startpoint, .Step = Step };
	}
	auto End() const {
		return Iterator{ .Cursor = Endpoint, .Step = Step };
	}
};