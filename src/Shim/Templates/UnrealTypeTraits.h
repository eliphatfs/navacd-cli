#pragma once

#include <type_traits>

template<typename T>
struct TIsTriviallyDestructible : std::is_trivially_destructible<T> {};

template<typename T>
struct TIsTriviallyCopyAssignable : std::is_trivially_copy_assignable<T> {};

template<typename T>
struct TIsTriviallyCopyConstructible : std::is_trivially_copy_constructible<T> {};

template<typename T>
inline constexpr bool TIsTriviallyDestructibleV = TIsTriviallyDestructible<T>::value;

template<typename T>
struct TIsZeroConstructType { enum { Value = false }; };

template<typename T>
struct THasDefaultValue { enum { Value = std::is_default_constructible_v<T> }; };

// Trivially relocatable - assume true for POD types
template<typename T>
struct TIsTriviallyRelocatable
{
	enum { Value = std::is_trivially_copyable_v<T> };
};
