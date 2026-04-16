#pragma once

// NavACD standalone: FTransform3d::ToInverseMatrixWithScale() returns a
// UE::Geometry::TMatrix3<double>.  The definition lives here so the
// out-of-line body is emitted after MatrixTypes.h is included.

#include "../GeometryCore/Public/MatrixTypes.h"
#include "Types.h"

inline UE::Geometry::TMatrix3<double> FTransform3d::ToInverseMatrixWithScale() const
{
	// Negative-space path is stubbed in NavACD; return identity.
	return UE::Geometry::TMatrix3<double>(1.0, 1.0, 1.0);
}
