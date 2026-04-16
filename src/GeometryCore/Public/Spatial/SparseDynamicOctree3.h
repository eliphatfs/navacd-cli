#pragma once
// NavACD standalone: minimal stub of FSparseDynamicOctree3.
//
// ConvexDecomposition3.cpp uses this octree purely to accelerate bounding-box
// overlap queries among convex parts.  The stub falls back to a linear scan
// over inserted (ObjectID, AABB) pairs, which is O(N^2) but correct for the
// small N encountered in convex-decomposition post-processing.  The real
// UE SparseDynamicOctree3 implementation depends on TBitArray, TSparseGrid3,
// FSparseOctreeCell, and other subsystems that are not present in this
// standalone extraction.

#include "BoxTypes.h"
#include "Containers.h"

namespace UE { namespace Geometry {

class FSparseDynamicOctree3
{
public:
	// Tunable knobs that real UE code sets; ignored by the linear-scan stub.
	double RootDimension = 1000.0;
	int32 MaxTreeDepth = 10;
	double MaxExpandFactor = 0.25;

	void InsertObject(int32 ObjectID, const FAxisAlignedBox3d& Bounds)
	{
		Entries.Add({ObjectID, Bounds});
	}

	void RemoveObject(int32 ObjectID)
	{
		for (int32 i = Entries.Num() - 1; i >= 0; --i)
		{
			if (Entries[i].ID == ObjectID) { Entries.RemoveAt(i); }
		}
	}

	void ReinsertObject(int32 ObjectID, const FAxisAlignedBox3d& Bounds)
	{
		RemoveObject(ObjectID);
		InsertObject(ObjectID, Bounds);
	}

	bool ContainsObject(int32 ObjectID) const
	{
		for (const FEntry& E : Entries) { if (E.ID == ObjectID) return true; }
		return false;
	}

	void RangeQuery(const FAxisAlignedBox3d& Query, TArray<int32>& Out) const
	{
		Out.Reset();
		for (const FEntry& E : Entries)
		{
			if (E.Bounds.Intersects(Query)) { Out.Add(E.ID); }
		}
	}

	template<typename PredicateType>
	void RangeQuery(const FAxisAlignedBox3d& Query, PredicateType Pred, TArray<int32>& Out) const
	{
		Out.Reset();
		for (const FEntry& E : Entries)
		{
			if (E.Bounds.Intersects(Query) && Pred(E.ID)) { Out.Add(E.ID); }
		}
	}

private:
	struct FEntry { int32 ID; FAxisAlignedBox3d Bounds; };
	TArray<FEntry> Entries;
};

}} // namespace UE::Geometry
