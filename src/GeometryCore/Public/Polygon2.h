#pragma once

// Stub for NavACD standalone build.  Delaunay2.h forward-declares
// UE::Geometry::TPolygon2<T> and references it in an overload that NavACD
// never calls.  Provide a minimal namespace-qualified template.
namespace UE { namespace Geometry {
template <typename T> class TPolygon2 {};
template <typename T> class TGeneralPolygon2 {};
}}
