#pragma once

#include <cstdint>
#include <cstddef>
#include <climits>
#include <limits>

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using SIZE_T = size_t;
using SSIZE_T = ptrdiff_t;

#define GEOMETRYCORE_API
#define GEOMETRYCORE_API_DEPRECATED

// UE Core numeric limits
#define MAX_int8 INT8_MAX
#define MAX_uint8 UINT8_MAX
#define MAX_int16 INT16_MAX
#define MAX_uint16 UINT16_MAX
#define MAX_int32 INT32_MAX
#define MAX_uint32 UINT32_MAX
#define MAX_int64 INT64_MAX
#define MAX_uint64 UINT64_MAX

// TNumericLimits - used by extracted IndexTypes.h as TNumericLimits<int>::Max()
template<typename T>
struct TNumericLimits
{
	static inline constexpr T Max() { return std::numeric_limits<T>::max(); }
	static inline constexpr T Min() { return std::numeric_limits<T>::lowest(); }
};

// Pragma macros
#define PRAGMA_DISABLE_DEPRECATION_WARNINGS
#define PRAGMA_ENABLE_DEPRECATION_WARNINGS
#define PRAGMA_DISABLE_UNREACHABLE_CODE_WARNINGS
#define PRAGMA_RESTORE_UNREACHABLE_CODE_WARNINGS
#define PRAGMA_ENABLE_UNREACHABLE_CODE_WARNINGS
#define PRAGMA_DISABLE_OPTIMIZATION
#define PRAGMA_ENABLE_OPTIMIZATION
#define PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS
#define PRAGMA_RESTORE_SHADOW_VARIABLE_WARNINGS

// UE_DEPRECATED
#define UE_DEPRECATED(Version, Message)
#define DEPRECATED(Version, Message)

// UE_NONCOPYABLE macro
#define UE_NONCOPYABLE(TypeName) \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete;
