#pragma once

#include <memory>

// TUniquePtr - wraps std::unique_ptr with UE-style IsValid()/Get() methods
template<typename T>
class TUniquePtr : public std::unique_ptr<T>
{
	using Base = std::unique_ptr<T>;
public:
	using Base::Base;
	using Base::operator=;

	TUniquePtr() : Base() {}
	TUniquePtr(std::nullptr_t) : Base(nullptr) {}
	TUniquePtr(Base&& Other) : Base(std::move(Other)) {}

	// UE-style methods
	bool IsValid() const { return static_cast<bool>(*this); }
	T* Get() { return Base::get(); }
	const T* Get() const { return Base::get(); }

	// Allow move from unique_ptr
	TUniquePtr& operator=(Base&& Other) { Base::operator=(std::move(Other)); return *this; }
};

template<typename T, typename... ArgsType>
TUniquePtr<T> MakeUnique(ArgsType&&... Args)
{
	return TUniquePtr<T>(std::make_unique<T>(std::forward<ArgsType>(Args)...));
}
