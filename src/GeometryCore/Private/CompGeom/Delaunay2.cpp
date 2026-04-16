// Copyright Epic Games, Inc. All Rights Reserved.
//
// NavACD standalone build: Delaunay2 is stubbed out.  ConvexDecomposition3
// uses it for planar hole-filling during splitting.  When Triangulate()
// returns false the caller falls back to its triangle-fan routine, so
// stubbing is safe and keeps the fallback path exercised.

#include "CompGeom/Delaunay2.h"

namespace UE { namespace Geometry {

struct FDelaunay2Connectivity { int Dummy = 0; };

bool FDelaunay2::Triangulate(TArrayView<const TVector2<double>>, TArrayView<const FIndex2i>)
{
	Result = EResult::Unknown;
	return false;
}
bool FDelaunay2::Triangulate(TArrayView<const TVector2<float>>, TArrayView<const FIndex2i>)
{
	Result = EResult::Unknown;
	return false;
}

bool FDelaunay2::ConstrainEdges(TArrayView<const TVector2<double>>, TArrayView<const FIndex2i>) { return false; }
bool FDelaunay2::ConstrainEdges(TArrayView<const TVector2<float>>, TArrayView<const FIndex2i>) { return false; }

bool FDelaunay2::Update(TArrayView<const TVector2<double>>, int32) { return false; }

TArray<FIndex3i> FDelaunay2::GetTriangles() const { return TArray<FIndex3i>(); }

void FDelaunay2::GetTrianglesAndAdjacency(TArray<FIndex3i>& Triangles, TArray<FIndex3i>& Adjacency) const
{
	Triangles.Reset();
	Adjacency.Reset();
}

bool FDelaunay2::GetFilledTriangles(TArray<FIndex3i>& TrianglesOut, TArrayView<const FIndex2i>, EFillMode) const
{
	TrianglesOut.Reset();
	return false;
}
bool FDelaunay2::GetFilledTriangles(TArray<FIndex3i>& TrianglesOut, TArrayView<const FIndex2i>, TArrayView<const FIndex2i>) const
{
	TrianglesOut.Reset();
	return false;
}

bool FDelaunay2::GetFilledTrianglesGeneralizedWinding(TArray<FIndex3i>& TrianglesOut, TArrayView<const TVector2<double>>, TArrayView<const FIndex2i>, EFillMode) const
{
	TrianglesOut.Reset();
	return false;
}
bool FDelaunay2::GetFilledTrianglesGeneralizedWinding(TArray<FIndex3i>& TrianglesOut, TArrayView<const TVector2<float>>, TArrayView<const FIndex2i>, EFillMode) const
{
	TrianglesOut.Reset();
	return false;
}

bool FDelaunay2::IsDelaunay(TArrayView<const FVector2f>, TArrayView<const FIndex2i>) const { return false; }
bool FDelaunay2::IsDelaunay(TArrayView<const FVector2d>, TArrayView<const FIndex2i>) const { return false; }

bool FDelaunay2::HasEdges(TArrayView<const FIndex2i>) const { return false; }

void FDelaunay2::FixDuplicatesOnEdge(FIndex2i&) {}

bool FDelaunay2::HasEdge(const FIndex2i&, bool) { return false; }

bool FDelaunay2::HasDuplicates() const { return false; }

int32 FDelaunay2::RemapIfDuplicate(int32 Index) const { return Index; }

TArray<TArray<FVector2d>> FDelaunay2::GetVoronoiCells(TArrayView<const FVector2d>, bool, FAxisAlignedBox2d, double) const
{
	return TArray<TArray<FVector2d>>();
}
TArray<TArray<FVector2f>> FDelaunay2::GetVoronoiCells(TArrayView<const FVector2f>, bool, FAxisAlignedBox2f, float) const
{
	return TArray<TArray<FVector2f>>();
}

}} // namespace UE::Geometry
