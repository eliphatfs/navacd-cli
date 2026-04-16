#pragma once

#include <functional>

// UE template function types mapped to std
template<typename T>
using TFunction = std::function<T>;

template<typename T>
using TFunctionRef = std::function<T>; // Note: not a perfect match (no reference semantics), but sufficient for our use

template<typename T>
using TUniqueFunction = std::function<T>;
