#pragma once

#include "CoreTypes.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

// UE assertion macros mapped to standard C++ assertions

#define check(Expression) assert(Expression)
#define checkf(Expression, Format, ...) assert(Expression)
#define checkSlow(Expression) // Disabled in standalone; was only for debug builds in UE
#define checkfSlow(Expression, Format, ...) // Disabled in standalone
#define verify(Expression) assert(Expression)
#define verifyf(Expression, Format, ...) assert(Expression)
#define verifySlow(Expression) // Disabled in standalone
#define checkNoEntry() assert(!"checkNoEntry: Should never reach this code path")
#define checkNoReentry() // Not easily mapped; skip
#define checkNoRecursion() // Not easily mapped; skip
#define ensure(Expression) (!(Expression) ? (std::fprintf(stderr, "Ensure failed: %s at %s:%d\n", #Expression, __FILE__, __LINE__), false) : true)
#define ensureMsgf(Expression, Format, ...) (!(Expression) ? (std::fprintf(stderr, "Ensure failed: %s at %s:%d\n", #Expression, __FILE__, __LINE__), false) : true)
#define unimplemented() assert(!"Unimplemented function called")
