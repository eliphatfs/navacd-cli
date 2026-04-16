#pragma once

#include <optional>
#include <utility>

// TOptional - wraps std::optional with UE-style API (IsSet, GetValue)
template<typename T>
class TOptional : public std::optional<T>
{
	using Base = std::optional<T>;
public:
	using Base::Base;
	using Base::operator=;

	TOptional() : Base() {}
	TOptional(const Base& Other) : Base(Other) {}
	TOptional(Base&& Other) : Base(std::move(Other)) {}

	// UE-style methods - explicitly call through this-> to help MSVC
	bool IsSet() const { return this->has_value(); }
	T& GetValue() & { return this->value(); }
	const T& GetValue() const& { return this->value(); }
	T&& GetValue() && { return std::move(this->value()); }
	const T&& GetValue() const&& { return std::move(this->value()); }

	// UE-style GetValue with default
	T Get(T DefaultValue) const { return this->has_value() ? **this : DefaultValue; }
};

// UE-style helpers
template<typename T>
inline TOptional<T> MakeOptional(T&& Value) { return TOptional<T>(std::forward<T>(Value)); }
