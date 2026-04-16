#pragma once

#include "CoreTypes.h"

// Serial stub for ParallelFor - will add OpenMP later
inline void ParallelFor(int32 Num, std::function<void(int32)> Body)
{
	for (int32 i = 0; i < Num; i++)
	{
		Body(i);
	}
}

// Overload with chunk size hint (ignored in serial mode)
inline void ParallelFor(int32 Num, int32 BatchSize, std::function<void(int32)> Body)
{
	for (int32 i = 0; i < Num; i++)
	{
		Body(i);
	}
}
