#pragma once
#include "UECompat.h"

// Polygon2 stub - minimal for NavACD
namespace UE { namespace Geometry {
class FPolygon2
{
public:
	TArray<FVector2d> Vertices;
	int32 VertexCount() const { return Vertices.Num(); }
	const FVector2d& operator[](int32 Idx) const { return Vertices[Idx]; }
	FVector2d& operator[](int32 Idx) { return Vertices[Idx]; }
};
}}
