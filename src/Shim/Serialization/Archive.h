#pragma once

#include "CoreTypes.h"

// Forward declarations for index types (defined in Types.h)
struct FIndex2i;
struct FIndex3i;
struct FIndex4i;

// Minimal archive stub - NavACD doesn't use serialization
class FArchive
{
public:
	bool IsLoading() const { return false; }
	bool IsSaving() const { return false; }

	// Stubs for DynamicVector.h serialization support
	bool UsingCustomVersion(const int32&) const { return false; }
	int32 CustomVer(const int32&) const { return 0; }
	int32 UEVer() const { return 0; }

	template<typename T>
	void Serialize(T* Data, int64 Count)
	{
		// No-op for standalone build
	}

	void CountBytes(SIZE_T, SIZE_T) {}

	template<typename T>
	void SerializeCompressedNew(T*, int64, const char*, const char*, int)
	{
		// No-op
	}
};

// Archive operators needed by IndexTypes.h
inline FArchive& operator<<(FArchive& Ar, int32&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, uint32&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, int64&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, uint64&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, float&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, double&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, bool&) { return Ar; }

// Archive operators for index types (no-op for standalone build)
inline FArchive& operator<<(FArchive& Ar, FIndex2i&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, FIndex3i&) { return Ar; }
inline FArchive& operator<<(FArchive& Ar, FIndex4i&) { return Ar; }

// Stubs for serialization version types referenced by DynamicVector.h, RefCountVector.h, etc.
struct FUE5MainStreamObjectVersion
{
	static const int32 GUID = 0;  // Stub GUID used with UsingCustomVersion/CustomVer
	static const int32 Type = 0;

	// Version enum values referenced by extracted code
	enum
	{
		DynamicMeshCompactedSerialization = 1,
		DynamicMeshAttributesWeightMapsAndNames = 2,
	};
};

enum EUnrealEngineObjectUE5Version { REMOVE_PACKAGE_SUMMARY_LOCALIZED_STRING = 0 };

// Compression stubs
#define NAME_Oodle FName()
#define COMPRESS_NoFlags 0
