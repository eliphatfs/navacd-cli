#pragma once

#include <utility>
#include <type_traits>
#include <algorithm>

// UE template utilities
template<typename T>
constexpr T&& MoveTemp(T& Obj) noexcept { return std::move(Obj); }

template<typename T>
constexpr T&& MoveTempIfPossible(T& Obj) noexcept { return std::move(Obj); }

template<typename T>
inline void Swap(T& A, T& B) { std::swap(A, B); }

template<typename T>
using TRemoveReference = std::remove_reference<T>;

template<typename T>
using TRemoveConst = std::remove_const<T>;

template<bool B, typename T = void>
using TEnableIf = std::enable_if<B, T>;

template<typename T>
inline constexpr bool TIsPointerV = std::is_pointer_v<T>;

template<typename T>
inline constexpr bool TIsReferenceV = std::is_reference_v<T>;

template<typename T>
struct TTypeTraits
{
	using ConstPointerType = const T*;
	using ConstReferenceType = const T&;
};

// Forward declare
template<typename T>
constexpr T&& Forward(std::remove_reference_t<T>& Obj) noexcept { return static_cast<T&&>(Obj); }

template<typename T>
constexpr T&& Forward(std::remove_reference_t<T>&& Obj) noexcept { return static_cast<T&&>(Obj); }

// NumBits
inline int32 NumBits(uint32 V)
{
#if defined(_MSC_VER)
	return __popcnt(V);
#else
	return __builtin_popcount(V);
#endif
}

// Alignment
template<uint32 Alignment>
struct TAlignedType
{
	alignas(Alignment) uint8 Pad[Alignment];
};
