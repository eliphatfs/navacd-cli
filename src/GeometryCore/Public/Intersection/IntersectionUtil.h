#pragma once
// NavACD standalone: minimal stub.  MeshAABBTree3 includes this for utility
// intersection helpers; NavACD's convex-decomposition pipeline does not call
// ray/triangle intersection queries, so the helper always returns false.

#include "VectorTypes.h"

namespace UE { namespace Geometry {
namespace IntersectionUtil
{
	template<typename RealType>
	inline bool RayTriangleTest(
		const UE::Math::TVector<RealType>& /*RayOrigin*/,
		const UE::Math::TVector<RealType>& /*RayDirection*/,
		const UE::Math::TVector<RealType>& /*A*/,
		const UE::Math::TVector<RealType>& /*B*/,
		const UE::Math::TVector<RealType>& /*C*/)
	{
		return false;
	}
}
}}
