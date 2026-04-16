#pragma once

// UE Compatibility Header - provides all UE types and APIs
// that the extracted GeometryCore source files depend on.
// This single header replaces dozens of UE includes.

#include <cstring>
#include <cstdlib>
#include <atomic>
#include "CoreTypes.h"
#include "Types.h"
#include "Containers.h"
#include "MathUtil.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Optional.h"
#include "Logging/LogMacros.h"
#include "HAL/Platform.h"
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

	template<typename RangeType, typename Transform, typename ValueType>
	inline ValueType TransformAccumulate(const RangeType& Range, Transform&& Xform, ValueType Init)
	{
		for (const auto& Elem : Range) Init += Xform(Elem);
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
	inline void Sort(RangeType&& Range)
	{
		std::sort(Range.begin(), Range.end());
	}

	template<typename RangeType, typename Predicate>
	inline void Sort(RangeType&& Range, Predicate Pred)
	{
		std::sort(Range.begin(), Range.end(), Pred);
	}

	// StableRemoveIf: stable variant of RemoveIf
	template<typename RangeType, typename Predicate>
	inline int32 StableRemoveIf(RangeType& Range, Predicate Pred)
	{
		auto It = std::stable_partition(Range.begin(), Range.end(), [&](const auto& V){ return !Pred(V); });
		int32 Removed = static_cast<int32>(Range.end() - It);
		Range.erase(It, Range.end());
		return Removed;
	}

	// UE's Algo::Unique returns int32 count of unique elements (does not erase)
	template<typename RangeType>
	inline int32 Unique(RangeType& Range)
	{
		auto It = std::unique(Range.begin(), Range.end());
		return static_cast<int32>(It - Range.begin());
	}

	template<typename RangeType, typename Predicate>
	inline int32 Unique(RangeType& Range, Predicate Pred)
	{
		auto It = std::unique(Range.begin(), Range.end(), Pred);
		return static_cast<int32>(It - Range.begin());
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

// Branch prediction macros
#ifndef LIKELY
#define LIKELY(x) __builtin_expect(!!(x), 1)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

// DO_GUARD_SLOW: UE debug macro; disabled in NavACD
#ifndef DO_GUARD_SLOW
#define DO_GUARD_SLOW 0
#endif

// Misc/DateTime stub
struct FDateTime
{
	double Seconds = 0.0;
	FDateTime() = default;
	FDateTime(double S) : Seconds(S) {}
	FDateTime& operator=(double S) { Seconds = S; return *this; }
	operator double() const { return Seconds; }

	static FDateTime Now()
	{
		auto NowTp = std::chrono::system_clock::now();
		auto Duration = NowTp.time_since_epoch();
		return FDateTime(std::chrono::duration<double>(Duration).count());
	}
	static FDateTime UtcNow() { return Now(); }
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
class FNameAsStringProxyArchive : public FArchive
{
public:
	FNameAsStringProxyArchive() = default;
	FNameAsStringProxyArchive(FArchive&) {}
};

// FArchive operator<< for FName (no-op stub) — forward-decl of FName follows below
class FName;
inline FArchive& operator<<(FArchive& Ar, FName&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, const FName&) { return Ar; }

// ENUM_CLASS_FLAGS: define bitwise ops on a scoped enum
#define ENUM_CLASS_FLAGS(Enum) \
	inline Enum operator|(Enum A, Enum B) { return (Enum)((__underlying_type(Enum))A | (__underlying_type(Enum))B); } \
	inline Enum operator&(Enum A, Enum B) { return (Enum)((__underlying_type(Enum))A & (__underlying_type(Enum))B); } \
	inline Enum operator^(Enum A, Enum B) { return (Enum)((__underlying_type(Enum))A ^ (__underlying_type(Enum))B); } \
	inline Enum operator~(Enum A) { return (Enum)(~(__underlying_type(Enum))A); } \
	inline Enum& operator|=(Enum& A, Enum B) { A = A | B; return A; } \
	inline Enum& operator&=(Enum& A, Enum B) { A = A & B; return A; } \
	inline Enum& operator^=(Enum& A, Enum B) { A = A ^ B; return A; }

// Global EnumHasAnyFlags / EnumHasAllFlags fallback
template<typename Enum>
inline bool EnumHasAnyFlags(Enum A, Enum B)
{
	using U = typename std::underlying_type<Enum>::type;
	return (static_cast<U>(A) & static_cast<U>(B)) != 0;
}
template<typename Enum>
inline bool EnumHasAllFlags(Enum A, Enum B)
{
	using U = typename std::underlying_type<Enum>::type;
	return (static_cast<U>(A) & static_cast<U>(B)) == static_cast<U>(B);
}

// UObject/NameTypes stub
class FName
{
public:
	FName() : Id(0) {}
	FName(const char*) : Id(0) {}
	FName(const FString&) : Id(0) {}
	FName(int) : Id(0) {} // for NAME_None == 0 conversions
	bool operator==(const FName& O) const { return Id == O.Id; }
	bool operator!=(const FName& O) const { return Id != O.Id; }
	bool IsNone() const { return Id == 0; }
	FString ToString() const { return FString(); }
private:
	int32 Id;
};

// EName enum stub
enum EName { NAME_None_Id = 0 };
inline const FName NAME_None = FName();

inline uint32 GetTypeHash(const FName& Name) { return Name.IsNone() ? 0u : 1u; }

namespace std
{
	template<> struct hash<FName>
	{
		size_t operator()(const FName& N) const noexcept { return GetTypeHash(N); }
	};
	template<> struct hash<FIndex2i>
	{
		size_t operator()(const FIndex2i& V) const noexcept { return GetTypeHash(V); }
	};
	template<> struct hash<FIndex3i>
	{
		size_t operator()(const FIndex3i& V) const noexcept { return GetTypeHash(V); }
	};
	template<> struct hash<FVector3i>
	{
		size_t operator()(const FVector3i& V) const noexcept
		{
			size_t H = std::hash<int>()(V.X);
			H ^= std::hash<int>()(V.Y) + 0x9e3779b9 + (H << 6) + (H >> 2);
			H ^= std::hash<int>()(V.Z) + 0x9e3779b9 + (H << 6) + (H >> 2);
			return H;
		}
	};
}

// FCStringAnsi stub — thin wrapper over C-runtime string helpers.  Used by
// the OBJ parser to pull ints/floats out of face-vertex tokens.
struct FCStringAnsi
{
	static int32 Atoi(const char* S) { return std::atoi(S); }
	static int64 Atoi64(const char* S) { return std::atoll(S); }
	static double Atod(const char* S) { return std::atof(S); }
	static float Atof(const char* S) { return static_cast<float>(std::atof(S)); }
	static size_t Strlen(const char* S) { return std::strlen(S); }
	static size_t Strcspn(const char* S, const char* Reject) { return std::strcspn(S, Reject); }
	static int Strcmp(const char* A, const char* B) { return std::strcmp(A, B); }
	static int Strncmp(const char* A, const char* B, size_t N) { return std::strncmp(A, B, N); }
	static const char* Strchr(const char* S, int C) { return std::strchr(S, C); }
};

// TAutoConsoleVariable / FAutoConsoleCommand stubs
template<typename T>
class TAutoConsoleVariable
{
public:
	TAutoConsoleVariable(const char*, T Default, const char* = nullptr, uint32 = 0) : Value(Default) {}
	T GetValueOnAnyThread() const { return Value; }
	T GetValueOnGameThread() const { return Value; }
	T* operator->() { return &Value; }
private:
	T Value;
};
class FAutoConsoleCommand
{
public:
	template<typename... Args>
	FAutoConsoleCommand(Args&&...) {}
};
class FAutoConsoleVariableRef
{
public:
	template<typename... Args>
	FAutoConsoleVariableRef(Args&&...) {}
};
namespace ECVF { enum Flags { Default = 0, Cheat = 0, ReadOnly = 0, Scalability = 0, SetByConsole = 0 }; }
#define ECVF_Default 0
#define ECVF_Cheat 0
#define ECVF_ReadOnly 0

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

// Containers/SparseSet stub (for ConvexHull3 which uses it as an index set)
// Backed by std::unordered_set<int32>; the template parameter is effectively
// ignored in the standalone build (ConvexHull3 only ever instantiates with T=int).
template<typename T>
class TSparseSet
{
	std::unordered_set<int32> Data;
public:
	using IteratorType = typename std::unordered_set<int32>::iterator;
	using ConstIteratorType = typename std::unordered_set<int32>::const_iterator;

	int32 Num() const { return static_cast<int32>(Data.size()); }
	bool IsEmpty() const { return Data.empty(); }
	bool Contains(int32 Idx) const { return Data.find(Idx) != Data.end(); }
	void Add(int32 Idx) { Data.insert(Idx); }
	void Add(int32 Idx, bool* bAlreadyInSet) { auto R = Data.insert(Idx); if (bAlreadyInSet) *bAlreadyInSet = !R.second; }
	int32 Remove(int32 Idx) { return static_cast<int32>(Data.erase(Idx)); }
	void Reset() { Data.clear(); }
	void Empty() { Data.clear(); }

	// Range-for support
	IteratorType begin() { return Data.begin(); }
	IteratorType end()   { return Data.end(); }
	ConstIteratorType begin() const { return Data.begin(); }
	ConstIteratorType end()   const { return Data.end(); }

	// UE-style iterator: implicit bool returns whether iterator is valid;
	// operator* returns the element
	struct FConstIterator
	{
		ConstIteratorType It;
		ConstIteratorType End;
		explicit operator bool() const { return It != End; }
		int32 operator*() const { return *It; }
		FConstIterator& operator++() { ++It; return *this; }
	};
	FConstIterator CreateConstIterator() const { return FConstIterator{Data.begin(), Data.end()}; }
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
	T* GetData() { return Data; }
	const T* GetData() const { return Data; }
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

// Templates/MemoryOps - CompareItems (used by DynamicVector.h)
template<typename T>
inline bool CompareItems(const T* A, const T* B, SIZE_T Count)
{
	for (SIZE_T i = 0; i < Count; ++i)
	{
		if (!(A[i] == B[i])) return false;
	}
	return true;
}

// Templates/PimplPtr stub — alias TUniquePtr (our own smart pointer that
// tolerates incomplete-type usage, unlike std::unique_ptr).
template<typename T>
using TPimplPtr = TUniquePtr<T>;

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
// Wrapper around std::mutex with UE-style Lock/Unlock methods.
struct FMutex
{
	std::mutex M;
	void Lock() { M.lock(); }
	void Unlock() { M.unlock(); }
	bool TryLock() { return M.try_lock(); }
};

// Async/Async stub
namespace Async
{
	// Simplified - not actually needed for NavACD
}

// HAL/PlatformCrt stub - nothing needed, standard C library available
// HAL/PlatformMath stub - functions already in FMath
// HAL/PlatformTime stub - already in Hal/Platform.h
// HAL/UnrealMemory stub
#include <cstdlib>
#include <cstring>
struct FMemory
{
	static void* Malloc(SIZE_T Count, uint32 Alignment = 0)
	{
		if (Alignment > alignof(std::max_align_t))
		{
			void* Ptr = nullptr;
			// Round size up to multiple of Alignment, as required by posix_memalign/aligned_alloc
			size_t Size = (Count + Alignment - 1) & ~(size_t(Alignment) - 1);
			if (posix_memalign(&Ptr, Alignment, Size) != 0)
			{
				return nullptr;
			}
			return Ptr;
		}
		return std::malloc(Count);
	}
	static void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = 0)
	{
		if (Alignment > alignof(std::max_align_t))
		{
			// No standard over-aligned realloc on Linux; allocate fresh and copy.
			void* NewPtr = Malloc(Count, Alignment);
			if (Original && NewPtr)
			{
				// Callers track their own size; copy min is not knowable here, so copy Count bytes.
				std::memcpy(NewPtr, Original, Count);
				std::free(Original);
			}
			return NewPtr;
		}
		return std::realloc(Original, Count);
	}
	static void Free(void* Original) { std::free(Original); }
	static void Memzero(void* Dest, SIZE_T Count) { memset(Dest, 0, Count); }
	static void Memcpy(void* Dest, const void* Src, SIZE_T Count) { memcpy(Dest, Src, Count); }
	static void Memset(void* Dest, uint8 Byte, SIZE_T Count) { memset(Dest, Byte, Count); }
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

// ========================================================================
// FMarchingCubes stub - NavACD does not exercise the marching-cubes path
// (it's only used for optional negative-space sampling).  The stub provides
// the member surface ConvexDecomposition3.cpp touches; Generate() is a no-op
// so the resulting mesh is empty and downstream code skips negative space.
// ========================================================================
namespace UE { namespace Geometry {
// Stub FMeshShapeGenerator base (forward-declared in DynamicMesh/DynamicMesh3.h)
class FMeshShapeGenerator
{
public:
	virtual ~FMeshShapeGenerator() = default;
};

enum class ERootfindingModes { SingleLerp, LerpSteps, Bisection };

// FMarchingCubes stub is defined after BoxTypes.h at the bottom of this header.
class FMarchingCubes;
}} // namespace UE::Geometry

// Add TArrayView and other container types to UE::Geometry namespace
// (can't do this in Types.h because they're defined in Containers.h)
namespace UE { namespace Geometry {
using ::TArrayView;
// FAxisAlignedBox3d is defined in UE::Geometry by BoxTypes.h (typedef TAxisAlignedBox3<double>)
// Note: FPlane3d/FSphere3d/FSphere3f/FHalfspace3d/FLine3d/FTriangle3d/FSegment3d
// are defined directly inside UE::Geometry by the corresponding headers
// (PlaneTypes.h, SphereTypes.h, etc.), so we do not alias them here.
}}

// Pull in the MatrixTypes-dependent FTransform3d::ToInverseMatrixWithScale() body.
#include "FTransformMatrixInterop.h"

// BoxTypes.h provides UE::Geometry::FAxisAlignedBox3d (= TAxisAlignedBox3<double>)
// which is used by the FMarchingCubes stub (defined below) and by a lot of the
// ported GeometryCore sources.  It also transitively pulls TransformTypes.h which
// defines UE::Geometry::FTransformSRT3d (= TTransformSRT3<double>).
#include "../GeometryCore/Public/BoxTypes.h"

// Re-expose UE::Geometry's transform typedefs at global scope so that the
// extracted GeometryCore sources that reference `FTransformSRT3d` without the
// namespace prefix continue to compile.
using FTransformSRT3d = UE::Geometry::FTransformSRT3d;
using FTransformSRT3f = UE::Geometry::FTransformSRT3f;

// FMarchingCubes stub definition — ConvexDecomposition3.cpp references this
// class directly (reads/writes Bounds, CubeSize, etc.), so we provide a
// minimal inert implementation.  The NavACD convex-decomposition pipeline
// does not exercise the marching-cubes surface generator.
namespace UE { namespace Geometry {
class FMarchingCubes : public FMeshShapeGenerator
{
public:
	TFunction<double(FVector3d)> Implicit;
	double IsoValue = 0;
	FAxisAlignedBox3d Bounds;
	double CubeSize = 0.1;
	bool bParallelCompute = true;
	ERootfindingModes RootMode = ERootfindingModes::Bisection;
	int32 RootModeSteps = 5;

	// Mesh output fields (kept empty in the stub)
	TArray<FVector3d> Vertices;
	TArray<FIndex3i> Triangles;
	TArray<FVector3d> Normals;

	void Generate() {}
	template<typename SeedsType>
	void GenerateContinuation(const SeedsType&) {}
};
}} // namespace UE::Geometry
