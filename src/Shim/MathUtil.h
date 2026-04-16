#pragma once

// This shim provides UE Core math types that the extracted GeometryCore/MathUtil.h depends on.
// The actual FMath, FMathd, FMathf, TMathUtil come from the extracted GeometryCore header.

#include "CoreTypes.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <limits>

// Forward to the real GeometryCore/Public/MathUtil.h for TMathUtil / FMathf / FMathd.
#include "../GeometryCore/Public/MathUtil.h"
