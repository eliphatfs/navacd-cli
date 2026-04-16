#pragma once
// NavACD standalone: simplified stub of TPointHashGrid3.
//
// Real UE impl uses a hashed cell grid (TMultiMap<FVector3i, PointDataType>)
// for O(1) point queries.  Here we fall back to a linear scan, which is
// correct but O(N) per query.  Call sites (MergeCoincidentMeshEdges,
// VertexConnectedComponents, PriorityOrderPoints) use this for:
//   - InsertPointUnsafe(value, position)
//   - FindPointsInBall(position, radius, callback)
//   - EnumeratePointsInBall(position, radius, callback)
//   - Reset(newCellSize)
//   - Reserve(n)

#include "CoreMinimal.h"
#include "VectorTypes.h"
#include "MathUtil.h"

namespace UE { namespace Geometry {

template<typename PointDataType, typename RealType>
class TPointHashGrid3
{
private:
	struct FEntry
	{
		PointDataType Value;
		UE::Math::TVector<RealType> Position;
	};
	TArray<FEntry> Entries;
	RealType CellSize = RealType(1);
	PointDataType InvalidValue;

public:
	TPointHashGrid3(RealType InCellSize, PointDataType InInvalid)
		: CellSize(FMath::Max(TMathUtil<RealType>::ZeroTolerance, InCellSize))
		, InvalidValue(InInvalid)
	{}

	void Reserve(int32 Num) { Entries.Reserve(Num); }

	void Reset(RealType NewCellSize)
	{
		Entries.Reset();
		CellSize = FMath::Max(TMathUtil<RealType>::ZeroTolerance, NewCellSize);
	}

	PointDataType GetInvalidValue() const { return InvalidValue; }

	// Insert (not thread-safe, but there is no contention in the NavACD
	// stub since the pipeline runs single-threaded).
	void InsertPointUnsafe(const PointDataType& Value, const UE::Math::TVector<RealType>& Position)
	{
		Entries.Add({Value, Position});
	}

	void InsertPoint(const PointDataType& Value, const UE::Math::TVector<RealType>& Position)
	{
		InsertPointUnsafe(Value, Position);
	}

	bool RemovePointUnsafe(const PointDataType& Value, const UE::Math::TVector<RealType>& /*Position*/)
	{
		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			if (Entries[i].Value == Value) { Entries.RemoveAt(i); return true; }
		}
		return false;
	}

	// Linear scan of all inserted points; invoke Func(PointData) for any
	// whose stored position lies within Radius of Query.
	template<typename FuncType>
	void FindPointsInBall(const UE::Math::TVector<RealType>& Query, RealType Radius, FuncType&& Func) const
	{
		const RealType R2 = Radius * Radius;
		for (const FEntry& E : Entries)
		{
			UE::Math::TVector<RealType> D = E.Position - Query;
			if (D.X*D.X + D.Y*D.Y + D.Z*D.Z <= R2)
			{
				Func(E.Value);
			}
		}
	}

	template<typename FuncType>
	void EnumeratePointsInBall(const UE::Math::TVector<RealType>& Query, RealType Radius, FuncType&& Func) const
	{
		FindPointsInBall(Query, Radius, std::forward<FuncType>(Func));
	}

	// Variant with distance functor + output array.  The distance functor is
	// passed the candidate value and returns a squared distance; values whose
	// distance <= Radius^2 are appended to OutPoints.
	template<typename DistFuncType>
	void FindPointsInBall(const UE::Math::TVector<RealType>& Query, RealType Radius,
		DistFuncType DistSqFunc, TArray<PointDataType>& OutPoints) const
	{
		const RealType R2 = Radius * Radius;
		for (const FEntry& E : Entries)
		{
			RealType D2 = DistSqFunc(E.Value);
			if (D2 <= R2)
			{
				OutPoints.Add(E.Value);
			}
		}
	}

	// Variant with distance functor + predicate; the predicate receives
	// (PointData, DistSq) and may return false to stop iteration.
	template<typename DistFuncType, typename PredFuncType>
	void EnumeratePointsInBall(const UE::Math::TVector<RealType>& Query, RealType Radius,
		DistFuncType DistSqFunc, PredFuncType Pred) const
	{
		const RealType R2 = Radius * Radius;
		for (const FEntry& E : Entries)
		{
			RealType D2 = DistSqFunc(E.Value);
			if (D2 <= R2)
			{
				if (!Pred(E.Value, D2)) return;
			}
		}
	}

	// Bounded-distance nearest-point query; returns InvalidValue if none in range.
	template<typename DistFunc>
	PointDataType FindNearestInRadius(const UE::Math::TVector<RealType>& Query, RealType Radius, DistFunc DistanceSqFunc) const
	{
		PointDataType Best = InvalidValue;
		RealType BestD2 = Radius * Radius;
		for (const FEntry& E : Entries)
		{
			RealType D2 = DistanceSqFunc(E.Value);
			if (D2 <= BestD2) { BestD2 = D2; Best = E.Value; }
		}
		return Best;
	}
};

// UE-style convenience typedefs
template<typename PointDataType>
using TPointHashGrid3d = TPointHashGrid3<PointDataType, double>;
template<typename PointDataType>
using TPointHashGrid3f = TPointHashGrid3<PointDataType, float>;

}} // namespace UE::Geometry
