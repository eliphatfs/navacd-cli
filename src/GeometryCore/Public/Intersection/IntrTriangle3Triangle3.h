#pragma once

// NavACD standalone: FIntrTriangle3Triangle3d is stubbed.  MeshAABBTree3
// references it for triangle/triangle intersection queries that the
// convex-decomposition pipeline does not actually exercise (the AABB-tree
// is only used to accelerate point-in-hull/winding queries).  The stub
// provides the member surface the protected header needs to compile, with
// Test()/Find() returning false so no intersection results are produced.

#include "TriangleTypes.h"

namespace UE { namespace Geometry {

enum class EIntersectionType
{
	Empty,
	Point,
	Segment,
	MultiSegment,
	Polygon,
	Plane,
	Unknown
};

enum class EIntersectionResult
{
	NotComputed,
	Intersects,
	NoIntersection,
	InvalidQuery
};

struct FIntrTriangle3Triangle3d
{
	FTriangle3d Triangle0;
	FTriangle3d Triangle1;

	int32 Quantity = 0;
	FVector3d Points[6] = {};
	EIntersectionType Type = EIntersectionType::Empty;
	EIntersectionResult Result = EIntersectionResult::NoIntersection;

	void SetTriangle0(const FTriangle3d& T) { Triangle0 = T; }
	void SetTriangle1(const FTriangle3d& T) { Triangle1 = T; }
	const FTriangle3d& GetTriangle0() const { return Triangle0; }
	const FTriangle3d& GetTriangle1() const { return Triangle1; }
	void SetResult(EIntersectionResult InResult) { Result = InResult; }
	void SetResult(bool bInResult) { Result = bInResult ? EIntersectionResult::Intersects : EIntersectionResult::NoIntersection; }

	bool Test() { return false; }
	bool Find() { Quantity = 0; Type = EIntersectionType::Empty; return false; }

	template<typename T1, typename T2>
	static bool Intersects(const T1&, const T2&) { return false; }
};

}}
