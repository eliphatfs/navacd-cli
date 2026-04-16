#pragma once

// This shim provides UE Core math types that the extracted GeometryCore/MathUtil.h depends on.
// The actual FMath, FMathd, FMathf, TMathUtil come from the extracted source file.

#include "CoreTypes.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <limits>
