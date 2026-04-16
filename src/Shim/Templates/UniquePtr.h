#pragma once

#include <memory>
#include <utility>

// TUniquePtr - UE-compat smart pointer.
//
// IMPORTANT: deliberately does NOT inherit from std::unique_ptr.  The issue
// with inheritance is that std::unique_ptr's destructor instantiates
// default_delete<T>::operator(), which static_asserts sizeof(T) > 0.  When the
// enclosing class (e.g. FDynamicMesh3) has a TUniquePtr<ForwardDeclaredT>
// member with an in-class `{}` initializer, the compiler eagerly instantiates
// the member's default ctor — which, for an inheriting TUniquePtr, pulls in
// the base destructor requiring complete type.
//
// Instead we hold a raw pointer and use a function-pointer deleter that's
// bound only when Reset() is called with a live pointer (i.e. in a TU that has
// the complete type).  Default-constructing TUniquePtr<IncompleteT> leaves the
// deleter null and is a no-op on destruction.
template<typename T>
class TUniquePtr
{
	T* Ptr = nullptr;
	void (*Deleter)(T*) = nullptr;

	template<typename U> friend class TUniquePtr;

public:
	TUniquePtr() = default;
	TUniquePtr(std::nullptr_t) {}

	explicit TUniquePtr(T* InPtr)
		: Ptr(InPtr)
		, Deleter(InPtr ? &DeleteComplete : nullptr)
	{}

	// Adopt a std::unique_ptr (requires complete type at the call site because
	// std::unique_ptr's dtor runs when the argument goes out of scope — but by
	// that point we've already stolen the pointer).
	TUniquePtr(std::unique_ptr<T>&& Other)
		: Ptr(Other.release())
		, Deleter(Ptr ? &DeleteComplete : nullptr)
	{}

	TUniquePtr(TUniquePtr&& Other) noexcept
		: Ptr(Other.Ptr), Deleter(Other.Deleter)
	{
		Other.Ptr = nullptr;
		Other.Deleter = nullptr;
	}

	// Derived-to-base conversion (e.g. TUniquePtr<Derived> -> TUniquePtr<Base>).
	template<typename U, typename = std::enable_if_t<
		std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>>>
	TUniquePtr(TUniquePtr<U>&& Other) noexcept
		: Ptr(static_cast<T*>(Other.Ptr))
		, Deleter(Other.Ptr ? &DeleteDerived<U> : nullptr)
	{
		Other.Ptr = nullptr;
		Other.Deleter = nullptr;
	}

	template<typename U, typename = std::enable_if_t<
		std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>>>
	TUniquePtr& operator=(TUniquePtr<U>&& Other) noexcept
	{
		DoDelete();
		Ptr = static_cast<T*>(Other.Ptr);
		Deleter = Other.Ptr ? &DeleteDerived<U> : nullptr;
		Other.Ptr = nullptr;
		Other.Deleter = nullptr;
		return *this;
	}

	TUniquePtr& operator=(TUniquePtr&& Other) noexcept
	{
		if (this != &Other)
		{
			DoDelete();
			Ptr = Other.Ptr;
			Deleter = Other.Deleter;
			Other.Ptr = nullptr;
			Other.Deleter = nullptr;
		}
		return *this;
	}

	TUniquePtr& operator=(std::nullptr_t) { DoDelete(); return *this; }

	TUniquePtr(const TUniquePtr&) = delete;
	TUniquePtr& operator=(const TUniquePtr&) = delete;

	~TUniquePtr() { DoDelete(); }

	// UE API
	bool IsValid() const { return Ptr != nullptr; }
	explicit operator bool() const { return Ptr != nullptr; }
	T* Get() const { return Ptr; }
	T& operator*() const { return *Ptr; }
	T* operator->() const { return Ptr; }

	void Reset(T* NewPtr = nullptr)
	{
		DoDelete();
		Ptr = NewPtr;
		Deleter = NewPtr ? &DeleteComplete : nullptr;
	}

	T* Release()
	{
		T* Result = Ptr;
		Ptr = nullptr;
		Deleter = nullptr;
		return Result;
	}

private:
	static void DeleteComplete(T* P) { delete P; }
	// Delete via the most-derived type so the correct destructor runs.
	template<typename U>
	static void DeleteDerived(T* P) { delete static_cast<U*>(P); }

	void DoDelete()
	{
		if (Ptr && Deleter) { Deleter(Ptr); }
		Ptr = nullptr;
		Deleter = nullptr;
	}
};

template<typename T, typename... ArgsType>
TUniquePtr<T> MakeUnique(ArgsType&&... Args)
{
	return TUniquePtr<T>(new T(std::forward<ArgsType>(Args)...));
}
