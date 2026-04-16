#pragma once

#include "CompGeom/ConvexDecomposition3.h"

namespace NavACD
{

// Run the Navigation-Driven Approximate Convex Decomposition algorithm
// Parameters:
//   Mesh: Input triangle mesh
//   MinRadiusFrac: Navigable space min radius, as fraction of longest bounding box axis
//   ToleranceFrac: Navigable space tolerance, as fraction of longest bounding box axis
//   bIgnoreUnreachableInternalSpace: If true, ignores unreachable internal pockets
//   CustomNavigablePositions: Optional additional navigable positions
UE::Geometry::FConvexDecomposition3 Run(
	const UE::Geometry::FDynamicMesh3& Mesh,
	double MinRadiusFrac,
	double ToleranceFrac,
	bool bIgnoreUnreachableInternalSpace,
	UE::Geometry::TArrayView<const FVector3d> CustomNavigablePositions = UE::Geometry::TArrayView<const FVector3d>());

}
