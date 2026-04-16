#pragma once

// UE Compatibility Header - provides all UE types and APIs
// that the extracted GeometryCore source files depend on.
// This single header replaces dozens of UE includes.

#include "CoreTypes.h"
#include "Types.h"
#include "Containers.h"
#include "MathUtil.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Optional.h"
#include "Logging/LogMacros.h"
#include "Hal/Platform.h"
#include "Async/ParallelFor.h"
#include "Async/TransactionallySafeMutex.h"
#include "Templates/Function.h"
#include "Templates/UniquePtr.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/UnrealTypeTraits.h"
#include "Serialization/Archive.h"

// ========================================================================
// Additional missing stubs that extracted source files reference
// ========================================================================

// Algo/ headers - simple wrappers
namespace Algo
{
	template<typename RangeType, typename ValueType>
	inline ValueType Accumulate(const RangeType& Range, ValueType Init)
	{
		for (const auto& Elem : Range) Init += Elem;
		return Init;
	}

	template<typename RangeType, typename Predicate>
	inline int32 RemoveIf(RangeType& Range, Predicate Pred)
	{
		auto It = std::remove_if(Range.begin(), Range.end(), Pred);
		int32 Removed = static_cast<int32>(Range.end() - It);
		Range.erase(It, Range.end());
		return Removed;
	}

	template<typename RangeType>
	inline void Reverse(RangeType& Range)
	{
		std::reverse(Range.begin(), Range.end());
	}

	template<typename RangeType>
	inline void Sort(RangeType& Range)
	{
		std::sort(Range.begin(), Range.end());
	}

	template<typename RangeType, typename Predicate>
	inline void Sort(RangeType& Range, Predicate Pred)
	{
		std::sort(Range.begin(), Range.end(), Pred);
	}

	template<typename RangeType>
	inline auto Unique(RangeType& Range) -> decltype(Range.begin())
	{
		return std::unique(Range.begin(), Range.end());
	}
}

// HAL stubs
namespace HAL
{
	// Empty namespace for HAL compatibility
}

// FCriticalSection - wrapper around std::mutex with UE-style Lock/Unlock
class FCriticalSection : public std::mutex
{
public:
	using std::mutex::mutex;
	void Lock() { lock(); }
	void Unlock() { unlock(); }
};

// FScopeLock - UE takes a pointer to FCriticalSection, unlike std::lock_guard which takes a reference
class FScopeLock
{
	FCriticalSection& MutexRef;
public:
	FScopeLock(FCriticalSection* InMutex) : MutexRef(*InMutex) { MutexRef.lock(); }
	~FScopeLock() { MutexRef.unlock(); }
	FScopeLock(const FScopeLock&) = delete;
	FScopeLock& operator=(const FScopeLock&) = delete;
};

// EngineDefines stubs
#define UE_LARGE_WORLD_MAX (3.402823466e+38)

// Misc/DateTime stub
struct FDateTime
{
	static double Now()
	{
		auto Now = std::chrono::system_clock::now();
		auto Duration = Now.time_since_epoch();
		return std::chrono::duration<double>(Duration).count();
	}
};

// Math/RandomStream stub
class FRandomStream
{
public:
	FRandomStream(int32 InSeed = 0) : Seed(InSeed) {}
	double GetFraction() { return static_cast<double>(std::rand()) / RAND_MAX; }
	int32 GetCurrentSeed() const { return Seed; }
private:
	int32 Seed;
};

// FColor is now defined in Types.h (before FLinearColor which uses it)

// TypeHash stubs
inline uint32 GetTypeHash(const FIndex2i& K) { return FIndex2iHash()(K); }
inline uint32 GetTypeHash(const FIndex3i& K) { return FIndex3iHash()(K); }
inline uint32 GetTypeHash(const FVector3d& V) { return static_cast<uint32>(FVector3dHash()(V)); }
inline uint32 GetTypeHash(int32 V) { return static_cast<uint32>(V); }
inline uint32 GetTypeHash(uint32 V) { return V; }
inline uint32 GetTypeHash(int64 V) { return static_cast<uint32>(V ^ (V >> 32)); }
inline uint32 GetTypeHash(uint64 V) { return static_cast<uint32>(V ^ (V >> 32)); }
inline uint32 GetTypeHash(float V) { return GetTypeHash(*reinterpret_cast<const uint32*>(&V)); }
inline uint32 GetTypeHash(double V) { return GetTypeHash(*reinterpret_cast<const uint64*>(&V)); }
inline uint32 GetTypeHash(const FString& S) { return static_cast<uint32>(std::hash<std::string>()(S)); }

// UObject version stubs - we don't need serialization
struct FCustomVersionRegistration { FCustomVersionRegistration(...) {} };

// Serialization/NameAsStringProxyArchive stub
class FNameAsStringProxyArchive : public FArchive {};

// UObject/NameTypes stub
class FName
{
public:
	FName() : Id(0) {}
	FName(const char*) : Id(0) {}
	FName(const FString&) : Id(0) {}
	bool operator==(const FName& O) const { return Id == O.Id; }
	bool operator!=(const FName& O) const { return Id != O.Id; }
	bool IsNone() const { return Id == 0; }
private:
	int32 Id;
};

// Containers/BitArray stub
class TBitArray
{
	std::vector<uint8> Data;
	int32 NumBits;
public:
	TBitArray() : NumBits(0) {}
	TBitArray(bool DefaultValue, int32 InNum) : Data((InNum + 7) / 8, DefaultValue ? 0xFF : 0), NumBits(InNum) {}
	int32 Num() const { return NumBits; }
	void SetNum(int32 InNum) { Data.resize((InNum + 7) / 8); NumBits = InNum; }
	bool operator[](int32 Idx) const { return (Data[Idx / 8] >> (Idx % 8)) & 1; }
	void Set(int32 Idx, bool Val) { if (Val) Data[Idx / 8] |= (1 << (Idx % 8)); else Data[Idx / 8] &= ~(1 << (Idx % 8)); }
};

// Containers/SparseSet stub
template<typename T>
class TSparseSet
{
	TMap<int32, T> Data;
public:
	int32 Num() const { return Data.Num(); }
	bool IsEmpty() const { return Data.IsEmpty(); }
	T* Find(int32 Idx) { return Data.Find(Idx); }
	const T* Find(int32 Idx) const { return Data.Find(Idx); }
	T& operator[](int32 Idx) { return Data.FindOrAdd(Idx); }
	const T& operator[](int32 Idx) const { return *Data.Find(Idx); }
	bool Contains(int32 Idx) const { return Data.Contains(Idx); }
	void Add(int32 Idx, const T& Val) { Data.FindOrAdd(Idx) = Val; }
	int32 Remove(int32 Idx) { return Data.Remove(Idx); }
	void Reset() { Data.Reset(); }
	void Empty() { Data.Empty(); }
};

// Containers/StaticArray stub
template<typename T, int32 N>
class TStaticArray
{
	T Data[N];
public:
	T& operator[](int32 Idx) { return Data[Idx]; }
	const T& operator[](int32 Idx) const { return Data[Idx]; }
	constexpr int32 Num() const { return N; }
};

// Containers/ContainerAllocationPolicies stubs
class FHeapAllocator {};
class FSparseArrayAllocator {};

// ProfilingDebugging/CpuProfilerTrace stub
#define TRACE_CPUPROFILER_EVENT_SCOPE(Text)
#define SCOPED_NAMED_EVENT(Name, Color)
#define SCOPED_NAMED_EVENT_F(Name, Color, Format, ...)

// Tasks/Task stub
namespace UE::Tasks
{
	inline void Launch(const char*, auto&&) {}
}

// Templates/TypeHash
template<typename T>
inline uint32 GetTypedHash(const T& V) { return GetTypeHash(V); }

// Templates/PimplPtr stub
template<typename T>
using TPimplPtr = std::unique_ptr<T>;

// Math/MathFwd - forward declarations already in Types.h

// Math/UnrealMath stubs
inline double Fmod(double X, double Y) { return std::fmod(X, Y); }

// Math/UnrealMathUtility stubs
struct FMathMatrix
{
	// Empty - math utility functions are in FMath namespace
};

// Polygon2 stub (needed by some code)
class FPolygon2
{
public:
	TArray<FVector2d> Vertices;
	int32 VertexCount() const { return Vertices.Num(); }
	const FVector2d& operator[](int32 Idx) const { return Vertices[Idx]; }
	FVector2d& operator[](int32 Idx) { return Vertices[Idx]; }
};

// Async/Mutex stub
using FMutex = std::mutex;

// Async/Async stub
namespace Async
{
	// Simplified - not actually needed for NavACD
}

// HAL/PlatformCrt stub - nothing needed, standard C library available
// HAL/PlatformMath stub - functions already in FMath
// HAL/PlatformTime stub - already in Hal/Platform.h
// HAL/UnrealMemory stub
struct FMemory
{
	static void* Malloc(SIZE_T Count, uint32 Alignment = 0) { return _aligned_malloc(Count, Alignment > 0 ? Alignment : sizeof(void*)); }
	static void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = 0) { return _aligned_realloc(Original, Count, Alignment > 0 ? Alignment : sizeof(void*)); }
	static void Free(void* Original) { _aligned_free(Original); }
	static void Memzero(void* Dest, SIZE_T Count) { memset(Dest, 0, Count); }
	static void Memcpy(void* Dest, const void* Src, SIZE_T Count) { memcpy(Dest, Src, Count); }
	static int32 Memcmp(const void* Buf1, const void* Buf2, SIZE_T Count) { return memcmp(Buf1, Buf2, Count); }
};

// HAL/IConsoleManager stub
class IConsoleVariable
{
public:
	void Set(const char*) {}
	int32 GetInt() const { return 0; }
	bool GetBool() const { return false; }
};

class IConsoleManager
{
public:
	static IConsoleManager& Get()
	{
		static IConsoleManager Instance;
		return Instance;
	}
	IConsoleVariable* FindConsoleVariable(const char*) { return nullptr; }
};

// FCrc stub - used by IndexTypes.h for GetTypeHash
struct FCrc
{
	static inline uint32 MemCrc_DEPRECATED(const void* Data, int32 Length, uint32 CRC = 0)
	{
		const uint8* Ptr = static_cast<const uint8*>(Data);
		for (int32 i = 0; i < Length; i++)
			CRC = CRC ^ (Ptr[i] << 16) ^ Ptr[i];
		return CRC;
	}
};

// TCanBulkSerialize stub - used by IndexTypes.h
template<typename T>
struct TCanBulkSerialize { static const bool Value = false; };

// Define the LogGeometryProcessing category
DEFINE_LOG_CATEGORY(LogGeometryProcessing)

// Define LogGeometry (from GeometryBase.h)
DEFINE_LOG_CATEGORY(LogGeometry)

// GEOMETRYCORE_API is defined as empty in CoreTypes.h

// Signal that TFunctionRef is provided by our shim
#define NAVACD_TFUNCTIONREF_PROVIDED

// Add TArrayView and other container types to UE::Geometry namespace
// (can't do this in Types.h because they're defined in Containers.h)
namespace UE { namespace Geometry {
using ::TArrayView;
using ::FAxisAlignedBox3d;
using ::FPlane3d;
using ::FPlane3f;
using ::FSphere3d;
using ::FSphere3f;
using ::FHalfspace3d;
using ::FLine3d;
using ::FTriangle3d;
using ::FSegment3d;
}}
