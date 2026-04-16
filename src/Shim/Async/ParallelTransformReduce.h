#pragma once

#include "ParallelFor.h"

// NavACD standalone: serial ParallelTransformReduce stub used by FastWinding.
// Applies Xform to each index and accumulates the result with Reduce.
template<typename OutType, typename XformType, typename ReduceType>
inline OutType ParallelTransformReduce(int32 Num, OutType Init, XformType&& Xform, ReduceType&& Reduce, int32 /*MinItemsPerTask*/ = 1)
{
	OutType Accum = Init;
	for (int32 i = 0; i < Num; ++i)
	{
		Accum = Reduce(Accum, Xform(i));
	}
	return Accum;
}
