#pragma once

#include "CoreTypes.h"

// EParallelForFlags stub - serial build, so flags are informational only
enum class EParallelForFlags : uint32
{
	None = 0,
	Unbalanced = 1 << 0,
	ForceSingleThread = 1 << 1,
	BackgroundPriority = 1 << 2,
	PumpRenderingThread = 1 << 3,
};

inline EParallelForFlags operator|(EParallelForFlags A, EParallelForFlags B)
{
	return static_cast<EParallelForFlags>(static_cast<uint32>(A) | static_cast<uint32>(B));
}
inline EParallelForFlags operator&(EParallelForFlags A, EParallelForFlags B)
{
	return static_cast<EParallelForFlags>(static_cast<uint32>(A) & static_cast<uint32>(B));
}

// Serial stub for ParallelFor - will add OpenMP later
template<typename FuncType>
inline void ParallelFor(int32 Num, FuncType&& Body)
{
	for (int32 i = 0; i < Num; i++)
	{
		Body(i);
	}
}

// With flags
template<typename FuncType>
inline void ParallelFor(int32 Num, FuncType&& Body, EParallelForFlags)
{
	for (int32 i = 0; i < Num; i++)
	{
		Body(i);
	}
}

// With bForceSingleThread bool (some UE code calls this variant)
template<typename FuncType>
inline void ParallelFor(int32 Num, FuncType&& Body, bool /*bForceSingleThread*/)
{
	for (int32 i = 0; i < Num; i++)
	{
		Body(i);
	}
}

// Overload with chunk size hint (ignored in serial mode)
template<typename FuncType>
inline void ParallelFor(int32 Num, int32 BatchSize, FuncType&& Body)
{
	(void)BatchSize;
	for (int32 i = 0; i < Num; i++)
	{
		Body(i);
	}
}
