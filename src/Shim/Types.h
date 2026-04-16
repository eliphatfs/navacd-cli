#pragma once

#include "CoreTypes.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <functional>
#include <string>
#include <cstdarg>

// ========================================================================
// Forward declarations
// ========================================================================
struct FIndex2i;
struct FIndex3i;
struct FIndex4i;
struct FIntVector;
struct FIntVector3;

// Forward-declare UE::Geometry::TMatrix3 so FTransform3d can return it.
namespace UE { namespace Geometry { template <typename RealType> struct TMatrix3; } }

// ========================================================================
// UE::Math namespace - primary vector/quaternion/transform templates
// In UE, FVector3d = UE::Math::TVector<double>, FVector3f = UE::Math::TVector<float>, etc.
// We define the templates first, then typedef the concrete types.
// ========================================================================
namespace UE { namespace Math {

template<typename T>
struct TVector2
{
	T X, Y;
	TVector2() : X(0), Y(0) {}
	TVector2(T InX, T InY) : X(InX), Y(InY) {}
	// Cross-type conversion
	template<typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
	explicit TVector2(const TVector2<U>& V) : X((T)V.X), Y((T)V.Y) {}
	T& operator[](int Idx) { return (&X)[Idx]; }
	const T& operator[](int Idx) const { return (&X)[Idx]; }
	TVector2 operator+(const TVector2& V) const { return TVector2(X+V.X, Y+V.Y); }
	TVector2 operator-(const TVector2& V) const { return TVector2(X-V.X, Y-V.Y); }
	TVector2 operator*(T S) const { return TVector2(X*S, Y*S); }
	TVector2 operator/(T S) const { return TVector2(X/S, Y/S); }
	TVector2& operator+=(const TVector2& V) { X+=V.X; Y+=V.Y; return *this; }
	TVector2& operator-=(const TVector2& V) { X-=V.X; Y-=V.Y; return *this; }
	bool operator==(const TVector2& V) const { return X==V.X && Y==V.Y; }
	bool operator!=(const TVector2& V) const { return X!=V.X || Y!=V.Y; }
	T SizeSquared() const { return X*X + Y*Y; }
	T Size() const { return std::sqrt(SizeSquared()); }
	void Normalize(T Tolerance = (T)1e-4) { T S = Size(); if (S > Tolerance) { X/=S; Y/=S; } }
	TVector2 GetNormalized(T Tolerance = (T)1e-4) const { TVector2 R = *this; R.Normalize(Tolerance); return R; }
	bool IsNearlyZero(T Tolerance = (T)1e-8) const { return SizeSquared() < Tolerance*Tolerance; }
	T Length() const { return Size(); }
	T SquaredLength() const { return SizeSquared(); }
	T Dot(const TVector2& V) const { return X*V.X + Y*V.Y; }
	static T DotProduct(const TVector2& A, const TVector2& B) { return A.X*B.X + A.Y*B.Y; }
	static T DistSquared(const TVector2& A, const TVector2& B) { return (A - B).SizeSquared(); }
	static TVector2 Zero() { return TVector2((T)0, (T)0); }
	static TVector2 One() { return TVector2((T)1, (T)1); }
	static TVector2 UnitX() { return TVector2((T)1, (T)0); }
	static TVector2 UnitY() { return TVector2((T)0, (T)1); }
	friend TVector2 operator*(T S, const TVector2& V) { return V * S; }
};

template<typename T>
struct TVector
{
	T X, Y, Z;
	TVector() : X(0), Y(0), Z(0) {}
	TVector(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ) {}
	explicit TVector(T InVal) : X(InVal), Y(InVal), Z(InVal) {}
	// Cross-type conversion
	template<typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
	explicit TVector(const TVector<U>& V) : X((T)V.X), Y((T)V.Y), Z((T)V.Z) {}
	T& operator[](int Idx) { return (&X)[Idx]; }
	const T& operator[](int Idx) const { return (&X)[Idx]; }
	TVector operator-() const { return TVector(-X, -Y, -Z); }
	TVector operator+(const TVector& V) const { return TVector(X+V.X, Y+V.Y, Z+V.Z); }
	TVector operator-(const TVector& V) const { return TVector(X-V.X, Y-V.Y, Z-V.Z); }
	// Uniform scalar add / subtract (UE supports V + double)
	TVector operator+(T S) const { return TVector(X+S, Y+S, Z+S); }
	TVector operator-(T S) const { return TVector(X-S, Y-S, Z-S); }
	TVector operator*(T S) const { return TVector(X*S, Y*S, Z*S); }
	TVector operator/(T S) const { return TVector(X/S, Y/S, Z/S); }
	TVector operator*(const TVector& V) const { return TVector(X*V.X, Y*V.Y, Z*V.Z); }
	TVector operator/(const TVector& V) const { return TVector(X/V.X, Y/V.Y, Z/V.Z); }
	TVector& operator+=(const TVector& V) { X+=V.X; Y+=V.Y; Z+=V.Z; return *this; }
	TVector& operator-=(const TVector& V) { X-=V.X; Y-=V.Y; Z-=V.Z; return *this; }
	TVector& operator*=(T S) { X*=S; Y*=S; Z*=S; return *this; }
	TVector& operator/=(T S) { X/=S; Y/=S; Z/=S; return *this; }
	TVector& operator*=(const TVector& V) { X*=V.X; Y*=V.Y; Z*=V.Z; return *this; }
	bool operator==(const TVector& V) const { return X==V.X && Y==V.Y && Z==V.Z; }
	bool operator!=(const TVector& V) const { return X!=V.X || Y!=V.Y || Z!=V.Z; }
	T SizeSquared() const { return X*X + Y*Y + Z*Z; }
	T Size() const { return std::sqrt(SizeSquared()); }
	T SquaredLength() const { return SizeSquared(); }
	T Length() const { return Size(); }
	bool Normalize(T Tolerance = (T)1e-8) { T S = Size(); if (S > Tolerance) { X/=S; Y/=S; Z/=S; return true; } return false; }
	TVector GetNormalized(T Tolerance = (T)1e-8) const { TVector R = *this; R.Normalize(Tolerance); return R; }
	bool IsNormalized() const { return std::abs(SizeSquared() - (T)1) < (T)1e-4; }
	bool IsNearlyZero(T Tolerance = (T)1e-8) const { return SizeSquared() < Tolerance*Tolerance; }
	bool ContainsNaN() const { return !std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z); }
	T Dot(const TVector& V) const { return X*V.X + Y*V.Y + Z*V.Z; }
	TVector Cross(const TVector& V) const { return TVector(Y*V.Z-Z*V.Y, Z*V.X-X*V.Z, X*V.Y-Y*V.X); }
	static T DotProduct(const TVector& A, const TVector& B) { return A.X*B.X + A.Y*B.Y + A.Z*B.Z; }
	static TVector CrossProduct(const TVector& A, const TVector& B) { return TVector(A.Y*B.Z-A.Z*B.Y, A.Z*B.X-A.X*B.Z, A.X*B.Y-A.Y*B.X); }
	static T DistSquared(const TVector& A, const TVector& B) { return (A-B).SizeSquared(); }
	static T Distance(const TVector& A, const TVector& B) { return std::sqrt(DistSquared(A, B)); }
	static T Dist(const TVector& A, const TVector& B) { return std::sqrt(DistSquared(A, B)); }
	bool AllComponentsEqual(T Tolerance = (T)1e-8) const { return std::abs(X - Y) <= Tolerance && std::abs(Y - Z) <= Tolerance; }
	static TVector Zero() { return TVector((T)0, (T)0, (T)0); }
	static TVector One() { return TVector((T)1, (T)1, (T)1); }
	static TVector UnitX() { return TVector((T)1, (T)0, (T)0); }
	static TVector UnitY() { return TVector((T)0, (T)1, (T)0); }
	static TVector UnitZ() { return TVector((T)0, (T)0, (T)1); }

	T GetAbsMax() const { return std::max({std::abs(X), std::abs(Y), std::abs(Z)}); }
		T GetAbsMin() const { return std::min({std::abs(X), std::abs(Y), std::abs(Z)}); }
	T GetMax() const { return std::max({X, Y, Z}); }
	T GetMin() const { return std::min({X, Y, Z}); }
	TVector GetAbs() const { return TVector(std::abs(X), std::abs(Y), std::abs(Z)); }
	TVector ClampAxes(T MinVal, T MaxVal) const { return TVector(std::clamp(X, MinVal, MaxVal), std::clamp(Y, MinVal, MaxVal), std::clamp(Z, MinVal, MaxVal)); }

	// UE static member names
	static const TVector ZeroVector;
	static const TVector OneVector;

	friend TVector operator*(T S, const TVector& V) { return V * S; }

	// Color aliases
	T& R() { return X; }
	const T& R() const { return X; }
	T& G() { return Y; }
	const T& G() const { return Y; }
	T& B() { return Z; }
	const T& B() const { return Z; }
};

template<typename T>
struct TVector4
{
	T X, Y, Z, W;
	TVector4() : X(0), Y(0), Z(0), W(0) {}
	TVector4(T InX, T InY, T InZ, T InW = (T)1) : X(InX), Y(InY), Z(InZ), W(InW) {}
	TVector4(const TVector<T>& V, T InW = (T)1) : X(V.X), Y(V.Y), Z(V.Z), W(InW) {}
	T& operator[](int Idx) { return (&X)[Idx]; }
	const T& operator[](int Idx) const { return (&X)[Idx]; }
};

template<typename T>
struct TSphere
{
	TVector<T> Center;
	T Radius;
	T W; // UE compatibility - sphere stores W component
	TSphere() : Center(), Radius(0), W(0) {}
	TSphere(const TVector<T>& InCenter, T InRadius) : Center(InCenter), Radius(InRadius), W(InRadius) {}
};

template<typename T>
struct TBox
{
	TVector<T> Min, Max;
	bool IsValid;
	TBox() : Min(), Max(), IsValid(false) {}
	TBox(const TVector<T>& InMin, const TVector<T>& InMax) : Min(InMin), Max(InMax), IsValid(true) {}
	TBox(const TVector<T>& InMin, const TVector<T>& InMax, bool bIsValid) : Min(InMin), Max(InMax), IsValid(bIsValid) {}
};

template<typename T>
struct TBox2
{
	TVector2<T> Min, Max;
	bool IsValid;
	TBox2() : Min(), Max(), IsValid(false) {}
	TBox2(const TVector2<T>& InMin, const TVector2<T>& InMax) : Min(InMin), Max(InMax), IsValid(true) {}
	TBox2(const TVector2<T>& InMin, const TVector2<T>& InMax, bool bIsValid) : Min(InMin), Max(InMax), IsValid(bIsValid) {}
};

template<typename T>
struct TPlane
{
	T X, Y, Z, W;
	TPlane() : X(0), Y(0), Z(1), W(0) {}
	TPlane(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW) {}
	TPlane(const TVector<T>& InNormal, T InW) : X(InNormal.X), Y(InNormal.Y), Z(InNormal.Z), W(InW) {}
	TPlane(const TVector<T>& InNormal, const TVector<T>& InPoint)
		: X(InNormal.X), Y(InNormal.Y), Z(InNormal.Z), W(TVector<T>::DotProduct(InNormal, InPoint)) {}
	TVector<T> GetNormal() const { return TVector<T>(X, Y, Z); }
};

template<typename T>
struct TRay
{
	TVector<T> Origin, Direction;
	TRay() : Origin(), Direction((T)1, (T)0, (T)0) {}
	TRay(const TVector<T>& InOrigin, const TVector<T>& InDirection) : Origin(InOrigin), Direction(InDirection.GetNormalized()) {}
	TVector<T> PointAt(T Param) const { return Origin + Direction * Param; }
};

template<typename T>
struct TTransform
{
	TVector<T> Translation;
	TVector<T> Scale3D;
	T RotX, RotY, RotZ, RotW;
	TTransform() : Translation(), Scale3D((T)1, (T)1, (T)1), RotX(0), RotY(0), RotZ(0), RotW((T)1) {}
	static TTransform Identity() { return TTransform(); }
	TVector<T> TransformPosition(const TVector<T>& P) const { return P * Scale3D + Translation; }
	TVector<T> TransformVector(const TVector<T>& V) const { return V * Scale3D; }
	TVector<T> TransformVectorNoScale(const TVector<T>& V) const { return V; }
	TVector<T> InverseTransformVectorNoScale(const TVector<T>& V) const { return V; }
	const TVector<T>& GetTranslation() const { return Translation; }
	const TVector<T>& GetScale3D() const { return Scale3D; }
};

}} // namespace UE::Math

// ========================================================================
// Concrete type aliases - matching UE's type definitions
// In UE: FVector3d = UE::Math::TVector<double>, etc.
// ========================================================================
using FVector3d = UE::Math::TVector<double>;
using FVector3f = UE::Math::TVector<float>;
using FVector2d = UE::Math::TVector2<double>;
using FVector2f = UE::Math::TVector2<float>;
using FVector4d = UE::Math::TVector4<double>;
using FVector4f = UE::Math::TVector4<float>;
using FRay3d = UE::Math::TRay<double>;
using FRay3f = UE::Math::TRay<float>;
using FSphere3d = UE::Math::TSphere<double>;
using FSphere3f = UE::Math::TSphere<float>;
using FBox = UE::Math::TBox<double>;
using FBox3d = UE::Math::TBox<double>;
using FBox3f = UE::Math::TBox<float>;
using FBox2d = UE::Math::TBox2<double>;
using FBox2f = UE::Math::TBox2<float>;
using FPlane = UE::Math::TPlane<double>;
using FPlane3f = UE::Math::TPlane<float>;

// Static member definitions for FVector3d and FVector3f
template<> inline const UE::Math::TVector<double> UE::Math::TVector<double>::ZeroVector(0, 0, 0);
template<> inline const UE::Math::TVector<double> UE::Math::TVector<double>::OneVector(1, 1, 1);
template<> inline const UE::Math::TVector<float> UE::Math::TVector<float>::ZeroVector(0, 0, 0);
template<> inline const UE::Math::TVector<float> UE::Math::TVector<float>::OneVector(1, 1, 1);

// ========================================================================
// FIntVector, FIntVector3 - needed early for FVector3i and FIndex3i conversions
// ========================================================================
struct FIntVector
{
	int32 X, Y, Z;
	FIntVector() : X(0), Y(0), Z(0) {}
	FIntVector(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}
	int32& operator[](int Idx) { return (&X)[Idx]; }
	const int32& operator[](int Idx) const { return (&X)[Idx]; }
	bool operator==(const FIntVector& O) const { return X==O.X && Y==O.Y && Z==O.Z; }
	bool operator!=(const FIntVector& O) const { return X!=O.X || Y!=O.Y || Z!=O.Z; }
};

struct FIntVector3 : public FIntVector
{
	FIntVector3() : FIntVector() {}
	FIntVector3(int32 InX, int32 InY, int32 InZ) : FIntVector(InX, InY, InZ) {}
};

// FVector3i - separate struct (not a UE::Math template)
struct FVector3i
{
	int X, Y, Z;
	FVector3i() : X(0), Y(0), Z(0) {}
	FVector3i(int InX, int InY, int InZ) : X(InX), Y(InY), Z(InZ) {}
	int& operator[](int Idx) { return (&X)[Idx]; }
	const int& operator[](int Idx) const { return (&X)[Idx]; }
	FVector3i operator+(const FVector3i& V) const { return FVector3i(X+V.X, Y+V.Y, Z+V.Z); }
	FVector3i operator-(const FVector3i& V) const { return FVector3i(X-V.X, Y-V.Y, Z-V.Z); }
	FVector3i operator*(int S) const { return FVector3i(X*S, Y*S, Z*S); }
	FVector3i operator/(int S) const { return FVector3i(X/S, Y/S, Z/S); }
	FVector3i& operator+=(const FVector3i& V) { X+=V.X; Y+=V.Y; Z+=V.Z; return *this; }
	bool operator==(const FVector3i& V) const { return X==V.X && Y==V.Y && Z==V.Z; }
	bool operator!=(const FVector3i& V) const { return X!=V.X || Y!=V.Y || Z!=V.Z; }
	int SizeSquared() const { return X*X + Y*Y + Z*Z; }
	operator FIntVector() const { return FIntVector(X, Y, Z); }
	explicit FVector3i(const FVector3d& V) : X((int)V.X), Y((int)V.Y), Z((int)V.Z) {}
	explicit FVector3i(const FVector3f& V) : X((int)V.X), Y((int)V.Y), Z((int)V.Z) {}
	explicit FVector3i(const FIntVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}
	static FVector3i Zero() { return FVector3i(0,0,0); }
	static FVector3i One() { return FVector3i(1,1,1); }
};

// FVector2i - 2D integer vector
struct FVector2i
{
	int32 X, Y;
	FVector2i() : X(0), Y(0) {}
	FVector2i(int32 InX, int32 InY) : X(InX), Y(InY) {}
	int32& operator[](int Idx) { return (&X)[Idx]; }
	const int32& operator[](int Idx) const { return (&X)[Idx]; }
	bool operator==(const FVector2i& V) const { return X==V.X && Y==V.Y; }
	bool operator!=(const FVector2i& V) const { return X!=V.X || Y!=V.Y; }
	explicit operator FVector2f() const { return FVector2f((float)X, (float)Y); }
	explicit operator FVector2d() const { return FVector2d((double)X, (double)Y); }
};

// FIntVector2 - 2D int vector (UE type)
struct FIntVector2
{
	int32 X, Y;
	FIntVector2() : X(0), Y(0) {}
	FIntVector2(int32 InX, int32 InY) : X(InX), Y(InY) {}
};

// Signal to IntVectorTypes.h that we've already defined these
#define NAVACD_INT_VECTOR_TYPES_PROVIDED

// Also put FVector3i/FVector2i/FIndex2i/FIndex3i/FIndex4i in UE::Geometry namespace
// so that code doing "using namespace UE::Geometry" finds them without ambiguity
// Note: other types (FFrame3d, FTransformSRT3d, etc.) are added later after they're defined
namespace UE { namespace Geometry {
using ::FVector3i;
using ::FVector2i;
using ::FIndex2i;
using ::FIndex3i;
using ::FIndex4i;
using ::FIntVector;
using ::FIntVector3;
}}

// FVector is FVector3d in UE for double precision
using FVector = FVector3d;
using FVector2D = FVector2d;

// FVector3f -> FVector3d conversion (needed by code)
// UE::Math::TVector has implicit conversions via constructors
// We need FVector3f(FVector3d) and FVector3d(FVector3f)
// Add specialization for FVector3f that can construct from FVector3d
// Actually these are the same template, just different T, so we need
// a converting constructor. Let's add it via a non-member approach.
// We'll handle this in the template above by adding a constructor.
// Actually, the template already handles this implicitly since both
// are TVector<T> with different T. We need an inter-type constructor.

// Add cross-type constructors. We need to specialize outside the class.
// Actually, let's just add free conversion functions and rely on the
// fact that FVector3f and FVector3d have the same layout.

// The UE code does: FVector3f(FVector3d) and FVector3d(FVector3f)
// Our templates don't have cross-type constructors. Add them.
// We can't add to the template easily, so we use free-standing conversions.

// FColor - UE color with 8-bit channels
struct FColor
{
	uint8 R, G, B, A;
	FColor() : R(0), G(0), B(0), A(255) {}
	FColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255) : R(InR), G(InG), B(InB), A(InA) {}
};

// FLinearColor - UE uses this with R,G,B,A members
struct FLinearColor
{
	double R, G, B, A;
	FLinearColor() : R(0), G(0), B(0), A(1) {}
	FLinearColor(double InR, double InG, double InB, double InA = 1.0) : R(InR), G(InG), B(InB), A(InA) {}
	FLinearColor(const FVector3d& V) : R(V.X), G(V.Y), B(V.Z), A(1.0) {}
	operator FVector3d() const { return FVector3d(R, G, B); }
		FColor ToFColor(bool bSRGB = false) const
		{
			return FColor(
				static_cast<uint8>(std::clamp(R, 0.0, 1.0) * 255.0),
				static_cast<uint8>(std::clamp(G, 0.0, 1.0) * 255.0),
				static_cast<uint8>(std::clamp(B, 0.0, 1.0) * 255.0),
				static_cast<uint8>(std::clamp(A, 0.0, 1.0) * 255.0));
		}
};

// ========================================================================
// FIndex2i, FIndex3i, FIndex4i
// These are defined here in the global namespace. IndexTypes.h is
// guarded with NAVACD_INDEX_TYPES_PROVIDED to avoid redefinition.
// ========================================================================
struct FIndex2i
{
	int32 A, B;
	constexpr FIndex2i() : A(-1), B(-1) {}
	constexpr FIndex2i(int32 InA, int32 InB) : A(InA), B(InB) {}
	int32& operator[](int Idx) { return (&A)[Idx]; }
	const int32& operator[](int Idx) const { return (&A)[Idx]; }
	constexpr bool operator==(const FIndex2i& O) const { return A==O.A && B==O.B; }
	constexpr bool operator!=(const FIndex2i& O) const { return A!=O.A || B!=O.B; }
	void Swap() { int32 T = A; A = B; B = T; }
	void Sort() { if (A > B) Swap(); }
	bool Contains(int32 V) const { return A == V || B == V; }
	int32 IndexOf(int32 V) const { return (A == V) ? 0 : ((B == V) ? 1 : -1); }
	// Given one of the two stored values, return the other.
	int32 OtherElement(int32 V) const { return (A == V) ? B : A; }
	static constexpr FIndex2i Zero() { return FIndex2i(0, 0); }
	static constexpr FIndex2i Invalid() { return FIndex2i(-1, -1); }
};

struct FIndex3i
{
	int32 A, B, C;
	constexpr FIndex3i() : A(-1), B(-1), C(-1) {}
	constexpr FIndex3i(int32 InA, int32 InB, int32 InC) : A(InA), B(InB), C(InC) {}
	int32& operator[](int Idx) { return (&A)[Idx]; }
	const int32& operator[](int Idx) const { return (&A)[Idx]; }
	constexpr bool operator==(const FIndex3i& O) const { return A==O.A && B==O.B && C==O.C; }
	constexpr bool operator!=(const FIndex3i& O) const { return A!=O.A || B!=O.B || C!=O.C; }
	operator FIntVector() const { return FIntVector(A, B, C); }
	static constexpr FIndex3i Zero() { return FIndex3i(0, 0, 0); }
	static constexpr FIndex3i Invalid() { return FIndex3i(-1, -1, -1); }
	bool Contains(int32 V) const { return A == V || B == V || C == V; }
	int32 IndexOf(int32 V) const {
		if (A == V) return 0; if (B == V) return 1; if (C == V) return 2; return -1;
	}
	// Cycle indices so element NewStart becomes the first element
	FIndex3i GetCycled(int32 NewStart) const
	{
		switch (NewStart % 3)
		{
			case 0: return FIndex3i(A, B, C);
			case 1: return FIndex3i(B, C, A);
			default: return FIndex3i(C, A, B);
		}
	}
};

struct FIndex4i
{
	int32 A, B, C, D;
	FIndex4i() : A(-1), B(-1), C(-1), D(-1) {}
	FIndex4i(int32 InA, int32 InB, int32 InC, int32 InD) : A(InA), B(InB), C(InC), D(InD) {}
	int32& operator[](int Idx) { return (&A)[Idx]; }
	const int32& operator[](int Idx) const { return (&A)[Idx]; }
	bool operator==(const FIndex4i& O) const { return A==O.A && B==O.B && C==O.C && D==O.D; }
	static FIndex4i Zero() { return FIndex4i(0, 0, 0, 0); }
	static FIndex4i Invalid() { return FIndex4i(-1, -1, -1, -1); }
	bool Contains(int32 V) const { return A == V || B == V || C == V || D == V; }
	int32 IndexOf(int32 V) const {
		if (A == V) return 0; if (B == V) return 1; if (C == V) return 2; if (D == V) return 3; return -1;
	}
};

// Signal to IndexTypes.h that we've already defined these types
#define NAVACD_INDEX_TYPES_PROVIDED

// IndexConstants - used by extracted code
namespace IndexConstants
{
	inline constexpr int InvalidID = -1;
}

// FIntVector/FIntVector3 defined earlier in the file

// ========================================================================
// FAxisAlignedBox3d — provided by UE::Geometry's BoxTypes.h (typedef of
// TAxisAlignedBox3<double>).  Global-scope usage relies on the UE::Geometry
// typedef being brought in via `using namespace UE::Geometry;`.
// ========================================================================

// ========================================================================
// FPlane3d
// ========================================================================
struct FPlane3d
{
	FVector3d Normal;
	double Constant;

	FPlane3d() : Normal(0, 0, 1), Constant(0) {}
	FPlane3d(const FVector3d& InNormal, double InConstant) : Normal(InNormal.GetNormalized()), Constant(InConstant) {}
	FPlane3d(double InX, double InY, double InZ, double InW) : Normal(InX, InY, InZ), Constant(InW) { Normal.Normalize(); }
	FPlane3d(const FVector3d& InNormal, const FVector3d& InPoint) : Normal(InNormal.GetNormalized()), Constant(FVector3d::DotProduct(Normal, InPoint)) {}
	// Plane from three points (CCW winding -> normal points to the "outside")
	FPlane3d(const FVector3d& A, const FVector3d& B, const FVector3d& C)
	{
		FVector3d N = (B - A).Cross(C - A);
		N.Normalize();
		Normal = N;
		Constant = FVector3d::DotProduct(Normal, A);
	}

	double DistanceTo(const FVector3d& P) const { return FVector3d::DotProduct(Normal, P) - Constant; }
	// Transform plane by an FTransform-like type (has TransformPosition / TransformVector).
	template<typename TransformType>
	void Transform(const TransformType& Xf)
	{
		FVector3d PointOnPlane = Normal * Constant;
		FVector3d XfPoint = Xf.TransformPosition(PointOnPlane);
		FVector3d XfNormal = Xf.TransformVector(Normal);
		XfNormal.Normalize();
		Normal = XfNormal;
		Constant = FVector3d::DotProduct(Normal, XfPoint);
	}
	int WhichSide(const FVector3d& P, double Tolerance = 1e-8) const
	{
		double D = DistanceTo(P);
		return (D > Tolerance) ? 1 : ((D < -Tolerance) ? -1 : 0);
	}

	bool IsValid() const { return !Normal.IsNearlyZero(); }
};

// ========================================================================
// FQuaterniond - standalone quaternion (UE's FQuat4d / FQuat4f)
// Note: The extracted Quaternion.h defines UE::Geometry::TQuaternion which is
// separate from UE::Math::TQuat. In UE, FQuat4d = UE::Math::TQuat<double>.
// ========================================================================
struct FQuaterniond
{
	double X, Y, Z, W;

	FQuaterniond() : X(0), Y(0), Z(0), W(1) {}
	FQuaterniond(double InX, double InY, double InZ, double InW) : X(InX), Y(InY), Z(InZ), W(InW) {}

	static FQuaterniond Identity() { return FQuaterniond(0, 0, 0, 1); }

	FVector3d RotateVector(const FVector3d& V) const
	{
		FQuaterniond Qv(V.X, V.Y, V.Z, 0);
		FQuaterniond Conjq(-X, -Y, -Z, W);
		FQuaterniond Result = (*this) * Qv * Conjq;
		return FVector3d(Result.X, Result.Y, Result.Z);
	}

	FVector3d UnrotateVector(const FVector3d& V) const
	{
		FQuaterniond Conjq(-X, -Y, -Z, W);
		return Conjq.RotateVector(V);
	}

	FQuaterniond operator*(const FQuaterniond& Q) const
	{
		return FQuaterniond(
			W*Q.X + X*Q.W + Y*Q.Z - Z*Q.Y,
			W*Q.Y - X*Q.Z + Y*Q.W + Z*Q.X,
			W*Q.Z + X*Q.Y - Y*Q.X + Z*Q.W,
			W*Q.W - X*Q.X - Y*Q.Y - Z*Q.Z);
	}

	void Normalize(double Tolerance = 1e-8)
	{
		double S = std::sqrt(X*X + Y*Y + Z*Z + W*W);
		if (S > Tolerance) { X/=S; Y/=S; Z/=S; W/=S; }
	}

	FQuaterniond GetNormalized() const { FQuaterniond R = *this; R.Normalize(); return R; }

	static FQuaterniond MakeFromAxisAngle(const FVector3d& Axis, double AngleRad)
	{
		double HalfAngle = AngleRad * 0.5;
		double S = std::sin(HalfAngle);
		FVector3d N = Axis.GetNormalized();
		return FQuaterniond(N.X*S, N.Y*S, N.Z*S, std::cos(HalfAngle));
	}

	// Rotator() - not used by NavACD convex decomposition path; return default
	struct FRotator Rotator() const;
};

// FQuat4d and FQuat4f - UE's math quaternion types
using FQuat4d = FQuaterniond;
struct FQuat4f
{
	float X, Y, Z, W;
	FQuat4f() : X(0), Y(0), Z(0), W(1) {}
	FQuat4f(float InX, float InY, float InZ, float InW) : X(InX), Y(InY), Z(InZ), W(InW) {}
	FQuat4f(const FQuaterniond& Q) : X((float)Q.X), Y((float)Q.Y), Z((float)Q.Z), W((float)Q.W) {}
	operator FQuaterniond() const { return FQuaterniond(X, Y, Z, W); }
};

// FQuat alias
using FQuat = FQuat4d;

// FOrientedBox stub - UE core type used only by operator conversions in OrientedBoxTypes.h
struct FOrientedBox
{
	FVector3d Center;
	FVector3d AxisX;
	FVector3d AxisY;
	FVector3d AxisZ;
	double ExtentX = 0;
	double ExtentY = 0;
	double ExtentZ = 0;
	FOrientedBox() = default;
};

// FRotator stub - minimal, just enough for Quaternion.h and TransformTypes.h
struct FRotator
{
	double Pitch, Yaw, Roll;
	FRotator() : Pitch(0), Yaw(0), Roll(0) {}
	FRotator(double InPitch, double InYaw, double InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}
	static FRotator MakeFromEuler(const FVector3d& Euler) { return FRotator(Euler.X, Euler.Y, Euler.Z); }
	FVector3d Euler() const { return FVector3d(Pitch, Yaw, Roll); }
};

// Out-of-line Rotator() for FQuaterniond (forward-declared above FRotator)
inline FRotator FQuaterniond::Rotator() const { return FRotator(); }

// ========================================================================
// FTransform3d / FTransform3f - UE engine transform types
// Used by Quaternion.h and TransformTypes.h conversion operators
// ========================================================================
struct FTransform3d
{
	FQuat4d Rotation;
	FVector3d Translation;
	FVector3d Scale3D;

	FTransform3d() : Rotation(), Translation(), Scale3D(1, 1, 1) {}
	FTransform3d(const FQuat4d& InRotation, const FVector3d& InTranslation, const FVector3d& InScale = FVector3d(1,1,1))
		: Rotation(InRotation), Translation(InTranslation), Scale3D(InScale) {}
	// Translation-only constructor (NavACD MeshTransforms::Translate uses this)
	explicit FTransform3d(const FVector3d& InTranslation)
		: Rotation(), Translation(InTranslation), Scale3D(1, 1, 1) {}

	const FQuat4d& GetRotation() const { return Rotation; }
	const FVector3d& GetTranslation() const { return Translation; }
	const FVector3d& GetScale3D() const { return Scale3D; }
	const FVector3d& GetScale() const { return Scale3D; }
	void SetRotation(const FQuat4d& R) { Rotation = R; }
	void SetTranslation(const FVector3d& T) { Translation = T; }
	void SetScale3D(const FVector3d& S) { Scale3D = S; }
	void SetScale(double S) { Scale3D = FVector3d(S, S, S); }

	FVector3d TransformPosition(const FVector3d& P) const
	{
		return Rotation.RotateVector(P * Scale3D) + Translation;
	}
	FVector3d TransformVector(const FVector3d& V) const
	{
		return Rotation.RotateVector(V * Scale3D);
	}
	FVector3d InverseTransformPosition(const FVector3d& P) const
	{
		FVector3d Scaled = P - Translation;
		FVector3d InvScale(1.0/Scale3D.X, 1.0/Scale3D.Y, 1.0/Scale3D.Z);
		return Rotation.UnrotateVector(Scaled) * InvScale;
	}
	FVector3d InverseTransformVector(const FVector3d& V) const
	{
		FVector3d InvScale(1.0/Scale3D.X, 1.0/Scale3D.Y, 1.0/Scale3D.Z);
		return Rotation.UnrotateVector(V) * InvScale;
	}

	static FTransform3d Identity() { return FTransform3d(); }
	double GetDeterminant() const { return Scale3D.X * Scale3D.Y * Scale3D.Z; }

	// Implicit conversion to UE::Math::TTransform<double> — UE-ported code
	// (PlaneTypes.h, TransformTypes.h, etc.) uses TTransform<RealType>& as the
	// parameter type for transform-accepting methods.  The stub TTransform is
	// value-only (no rotation support), so the conversion drops rotation but
	// preserves translation and scale.
	operator UE::Math::TTransform<double>() const
	{
		UE::Math::TTransform<double> R;
		R.Translation = Translation;
		R.Scale3D = Scale3D;
		return R;
	}

	// NavACD stub: negative-space path uses CoveringToLocalSpace *= transform.ToInverseMatrixWithScale().
	// Return a default-constructed UE::Geometry::TMatrix3<double> (forward-declared at top of this file).
	// Defined inline so it's emitted only where callers instantiate it AFTER MatrixTypes.h is available.
	inline UE::Geometry::TMatrix3<double> ToInverseMatrixWithScale() const;
};

struct FTransform3f
{
	FQuat4f Rotation;
	FVector3f Translation;
	FVector3f Scale3D;

	FTransform3f() : Rotation(), Translation(), Scale3D(1, 1, 1) {}
	FTransform3f(const FQuat4f& InRotation, const FVector3f& InTranslation, const FVector3f& InScale = FVector3f(1,1,1))
		: Rotation(InRotation), Translation(InTranslation), Scale3D(InScale) {}

	const FQuat4f& GetRotation() const { return Rotation; }
	const FVector3f& GetTranslation() const { return Translation; }
	const FVector3f& GetScale3D() const { return Scale3D; }
};

// FTransform - UE alias for FTransform3d
using FTransform = FTransform3d;

// ========================================================================
// FTransformSRT3d is provided by UE::Geometry (TTransformSRT3<double>) via
// TransformTypes.h.  We intentionally do NOT alias it at global scope; the
// UE::Geometry typedef has a richer API (SetScale(FVector), rotation, etc.)
// and we want it to be the one visible through `using namespace UE::Geometry`.
// ========================================================================

// ========================================================================
// FFrame3d
// ========================================================================
struct FFrame3d
{
	FVector3d Origin;
	FVector3d RotationX, RotationY, RotationZ;

	FFrame3d() : Origin(0,0,0), RotationX(1,0,0), RotationY(0,1,0), RotationZ(0,0,1) {}
	FFrame3d(const FVector3d& InOrigin, const FVector3d& InX, const FVector3d& InY, const FVector3d& InZ)
		: Origin(InOrigin), RotationX(InX), RotationY(InY), RotationZ(InZ) {}
	FFrame3d(const FVector3d& InOrigin, const FQuaterniond& InRotation)
		: Origin(InOrigin), RotationX(InRotation.RotateVector(FVector3d(1,0,0))),
		  RotationY(InRotation.RotateVector(FVector3d(0,1,0))),
		  RotationZ(InRotation.RotateVector(FVector3d(0,0,1))) {}

	FVector3d ToFramePoint(const FVector3d& P) const
	{
		FVector3d V = P - Origin;
		return FVector3d(FVector3d::DotProduct(V, RotationX), FVector3d::DotProduct(V, RotationY), FVector3d::DotProduct(V, RotationZ));
	}

	FVector3d FromFramePoint(const FVector3d& P) const
	{
		return Origin + RotationX * P.X + RotationY * P.Y + RotationZ * P.Z;
	}

	FVector3d ToFrameVector(const FVector3d& V) const
	{
		return FVector3d(FVector3d::DotProduct(V, RotationX), FVector3d::DotProduct(V, RotationY), FVector3d::DotProduct(V, RotationZ));
	}

	FVector3d FromFrameVector(const FVector3d& V) const
	{
		return RotationX * V.X + RotationY * V.Y + RotationZ * V.Z;
	}

	void ConstructFrameZ(const FVector3d& InZ)
	{
		RotationZ = InZ.GetNormalized();
		if (std::abs(RotationZ.X) < 0.9)
			RotationX = FVector3d::CrossProduct(FVector3d(1,0,0), RotationZ).GetNormalized();
		else
			RotationX = FVector3d::CrossProduct(FVector3d(0,1,0), RotationZ).GetNormalized();
		RotationY = FVector3d::CrossProduct(RotationZ, RotationX).GetNormalized();
	}
};

// ========================================================================
// FMatrix3d - 3x3 matrix (row-major)
// ========================================================================
struct FMatrix3d
{
	double M[3][3];

	FMatrix3d()
	{
		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) M[i][j] = (i==j) ? 1.0 : 0.0;
	}

	static FMatrix3d Identity() { return FMatrix3d(); }
	static FMatrix3d Zero() { FMatrix3d R; for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) R.M[i][j] = 0; return R; }

	FVector3d operator*(const FVector3d& V) const
	{
		return FVector3d(
			M[0][0]*V.X + M[0][1]*V.Y + M[0][2]*V.Z,
			M[1][0]*V.X + M[1][1]*V.Y + M[1][2]*V.Z,
			M[2][0]*V.X + M[2][1]*V.Y + M[2][2]*V.Z);
	}

	FMatrix3d operator*(const FMatrix3d& O) const
	{
		FMatrix3d R;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
			{
				R.M[i][j] = 0;
				for (int k = 0; k < 3; k++) R.M[i][j] += M[i][k] * O.M[k][j];
			}
		return R;
	}

	FMatrix3d Transpose() const
	{
		FMatrix3d R;
		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) R.M[i][j] = M[j][i];
		return R;
	}

	double Determinant() const
	{
		return M[0][0]*(M[1][1]*M[2][2]-M[1][2]*M[2][1])
		     - M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
		     + M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
	}

	const double* operator[](int Row) const { return M[Row]; }
	double* operator[](int Row) { return M[Row]; }

	double operator()(int Row, int Col) const { return M[Row][Col]; }
	double& operator()(int Row, int Col) { return M[Row][Col]; }

	// Row accessors used by Quaternion.h
	FVector3d Row0, Row1, Row2;

	// FMatrix-style helpers (NavACD: negative-space path is stubbed so
	// these only need to compile; TransformPosition applies the rotation
	// portion only which is sufficient to yield *some* FVector3d output).
	FVector3d TransformPosition(const FVector3d& P) const { return (*this) * P; }
	FVector3d TransformVector(const FVector3d& V) const { return (*this) * V; }

	FMatrix3d& operator*=(const FMatrix3d& O) { *this = (*this) * O; return *this; }

	// Accept any other matrix-like type (e.g. UE::Geometry::TMatrix3<double>).  The
	// NavACD negative-space path is stubbed, so this is a no-op that keeps
	// the caller compiling.
	template<typename OtherMat,
		typename = std::enable_if_t<!std::is_same_v<OtherMat, FMatrix3d>>>
	FMatrix3d& operator*=(const OtherMat& /*O*/)
	{
		return *this;
	}
};

// ========================================================================
// FIntVector, FIntVector3
// ========================================================================
// FIntVector/FIntVector3 defined earlier in the file

// ========================================================================
// FSphere
// ========================================================================
struct FSphere
{
	FVector3d Center;
	double Radius;
	FSphere() : Center(0,0,0), Radius(0) {}
	FSphere(const FVector3d& InCenter, double InRadius) : Center(InCenter), Radius(InRadius) {}
};

// ========================================================================
// FHalfspace3d
// ========================================================================
struct FHalfspace3d
{
	FVector3d Normal;
	double Constant;
	FHalfspace3d() : Normal(0,0,1), Constant(0) {}
	FHalfspace3d(const FVector3d& InNormal, double InConstant) : Normal(InNormal.GetNormalized()), Constant(InConstant) {}
	static FHalfspace3d FromPlane(const FPlane3d& Plane)
	{
		return FHalfspace3d(Plane.Normal, Plane.Constant);
	}
};

// ========================================================================
// FLine3d
// ========================================================================
struct FLine3d
{
	FVector3d Origin, Direction;
	FLine3d() : Origin(0,0,0), Direction(1,0,0) {}
	FLine3d(const FVector3d& InOrigin, const FVector3d& InDirection) : Origin(InOrigin), Direction(InDirection.GetNormalized()) {}
};

// ========================================================================
// FTriangle3d
// ========================================================================
struct FTriangle3d
{
	FVector3d V[3];
	FTriangle3d() {}
	FTriangle3d(const FVector3d& A, const FVector3d& B, const FVector3d& C) { V[0]=A; V[1]=B; V[2]=C; }
	FVector3d& operator[](int Idx) { return V[Idx]; }
	const FVector3d& operator[](int Idx) const { return V[Idx]; }
	double Area() const { return FVector3d::CrossProduct(V[1]-V[0], V[2]-V[0]).Size() * 0.5; }
	FVector3d Normal() const { return FVector3d::CrossProduct(V[1]-V[0], V[2]-V[0]).GetNormalized(); }
	FVector3d Centroid() const { return (V[0]+V[1]+V[2]) / 3.0; }
};

// ========================================================================
// FSegment3d
// ========================================================================
struct FSegment3d
{
	FVector3d Start, End;
	FSegment3d() : Start(0,0,0), End(0,0,0) {}
	FSegment3d(const FVector3d& InStart, const FVector3d& InEnd) : Start(InStart), End(InEnd) {}
	FVector3d Center() const { return (Start + End) * 0.5; }
	double Length() const { return FVector3d::Distance(Start, End); }
	FVector3d Direction() const { return (End - Start).GetNormalized(); }
};

// ========================================================================
// UE-compatible constants and macros
// ========================================================================
#define UE_DOUBLE_KINDA_SMALL_NUMBER (1e-4)
#define UE_KINDA_SMALL_NUMBER (1e-3f)
#define UE_SMALL_NUMBER (1e-8f)
#define UE_DOUBLE_SMALL_NUMBER (1e-8)
#define SMALL_NUMBER (1e-8f)
#define MAX_dbl DBL_MAX
#define MAX_flt FLT_MAX
#define LARGE_WORLD_MAX (3.402823466e+38)

// UE string/type macros
using TCHAR = char;
using ANSICHAR = char;
#define TEXT(x) x
#define ANSI_TO_TCHAR(x) x
#define TCHAR_TO_ANSI(x) x
#define TEXT_FMT "%s"

// FString with operator* support (UE uses *FString to get const TCHAR*)
struct FString : public std::string
{
	using std::string::string;
	FString(const std::string& S) : std::string(S) {}
	FString(std::string&& S) noexcept : std::string(std::move(S)) {}
	const char* operator*() const { return c_str(); }
	static FString Printf(const char* Fmt, ...)
	{
		char Buf[4096];
		va_list Args;
		va_start(Args, Fmt);
		vsnprintf(Buf, sizeof(Buf), Fmt, Args);
		va_end(Args);
		return FString(Buf);
	}
};

// Matrix types used by some code
using FMatrix = FMatrix3d;

// Free helper functions for vector operations
inline double VectorDot(const FVector3d& A, const FVector3d& B) { return FVector3d::DotProduct(A, B); }
inline FVector3d VectorCross(const FVector3d& A, const FVector3d& B) { return FVector3d::CrossProduct(A, B); }
inline FVector3d VectorNormalize(const FVector3d& V) { return V.GetNormalized(); }
inline double VectorDotDouble(const FVector3d& A, const FVector3d& B) { return FVector3d::DotProduct(A, B); }

// EAllowShrinking enum
enum class EAllowShrinking
{
	Yes,
	No
};

// TStructOnScope stub
struct FStructOnScope { };

// ForceInit enum
enum EForceInit { ForceInit };

// ========================================================================
// FMath namespace - basic math utilities
// The extracted MathUtil.h provides TMathUtil<RealType>, FMathd, FMathf
// ========================================================================
namespace FMath
{
	inline double Abs(double V) { return std::abs(V); }
	inline float Abs(float V) { return std::abs(V); }
	inline int32 Abs(int32 V) { return std::abs(V); }
	inline double Sqrt(double V) { return std::sqrt(V); }
	inline float Sqrt(float V) { return std::sqrt(V); }
	inline double InvSqrt(double V) { return 1.0 / std::sqrt(V); }
	template<typename T> inline T Min(const T& A, const T& B) { return std::min(A, B); }
	template<typename T> inline T Max(const T& A, const T& B) { return std::max(A, B); }
	// Mixed-type Min/Max: promote both operands to their common type
	template<typename T1, typename T2,
	         typename C = typename std::common_type<T1, T2>::type,
	         typename = std::enable_if_t<!std::is_same<T1, T2>::value>>
	inline C Min(T1 A, T2 B) { return std::min(static_cast<C>(A), static_cast<C>(B)); }
	template<typename T1, typename T2,
	         typename C = typename std::common_type<T1, T2>::type,
	         typename = std::enable_if_t<!std::is_same<T1, T2>::value>>
	inline C Max(T1 A, T2 B) { return std::max(static_cast<C>(A), static_cast<C>(B)); }
	template<typename T> inline T DegreesToRadians(T Deg) { return Deg * (T)(3.14159265358979323846 / 180.0); }
	template<typename T> inline T RadiansToDegrees(T Rad) { return Rad * (T)(180.0 / 3.14159265358979323846); }
	inline double Floor(double V) { return std::floor(V); }
	inline float Floor(float V) { return std::floor(V); }
	inline double Ceil(double V) { return std::ceil(V); }
	inline float Ceil(float V) { return std::ceil(V); }
	template<typename T> inline T Clamp(const T& V, const T& MinV, const T& MaxV) { return std::clamp(V, MinV, MaxV); }
	template<typename T> inline T Lerp(const T& A, const T& B, double Frac) { return A + (B - A) * Frac; }
	inline int32 CeilToInt(double V) { return static_cast<int32>(std::ceil(V)); }
	inline int32 FloorToInt(double V) { return static_cast<int32>(std::floor(V)); }
	inline int32 RoundToInt(double V) { return static_cast<int32>(std::round(V)); }
	inline bool IsNaN(double V) { return std::isnan(V); }
	inline bool IsFinite(double V) { return std::isfinite(V); }
	inline double FRand() { return static_cast<double>(std::rand()) / RAND_MAX; }
	inline int32 RandRange(int32 MinV, int32 MaxV) { return MinV + std::rand() % (MaxV - MinV + 1); }
	inline double Atan2(double Y, double X) { return std::atan2(Y, X); }
	inline double Sin(double V) { return std::sin(V); }
	inline double Cos(double V) { return std::cos(V); }
	inline double Acos(double V) { return std::acos(V); }
	template<typename T> inline int32 Sign(const T& V) { return (V > 0) ? 1 : ((V < 0) ? -1 : 0); }
	inline double Exp(double V) { return std::exp(V); }
	inline double Pow(double Base, double Exponent) { return std::pow(Base, Exponent); }
	inline double Log(double V) { return std::log(V); }
	inline double Fmod(double X, double Y) { return std::fmod(X, Y); }
	inline double Fractional(double V) { double I; return std::modf(V, &I); }
	inline double TruncToDouble(double V) { return static_cast<double>(static_cast<int64>(V)); }
	inline bool IsNearlyZero(double V, double Tolerance = 1e-8) { return std::abs(V) <= Tolerance; }
	inline bool IsNearlyEqual(double A, double B, double Tolerance = 1e-8) { return std::abs(A - B) <= Tolerance; }
	template<typename T> inline T Max3(const T& A, const T& B, const T& C) { return std::max(A, std::max(B, C)); }
	template<typename T> inline T Min3(const T& A, const T& B, const T& C) { return std::min(A, std::min(B, C)); }
	// Max3Index: index (0/1/2) of the largest of three values.
	template<typename T> inline int32 Max3Index(const T& A, const T& B, const T& C)
	{
		return (A >= B) ? (A >= C ? 0 : 2) : (B >= C ? 1 : 2);
	}
	template<typename T> inline int32 Min3Index(const T& A, const T& B, const T& C)
	{
		return (A <= B) ? (A <= C ? 0 : 2) : (B <= C ? 1 : 2);
	}
	// Mixed-type Clamp: promote all to common type
	template<typename T1, typename T2, typename T3,
	         typename C = typename std::common_type<T1, T2, T3>::type,
	         typename = std::enable_if_t<!(std::is_same<T1, T2>::value && std::is_same<T2, T3>::value)>>
	inline C Clamp(T1 V, T2 MinV, T3 MaxV)
	{
		return std::clamp(static_cast<C>(V), static_cast<C>(MinV), static_cast<C>(MaxV));
	}
	inline int32 CeilToInt32(double V) { return static_cast<int32>(std::ceil(V)); }
	inline int32 CeilToInt32(float V)  { return static_cast<int32>(std::ceil(V)); }
	inline int32 FloorToInt32(double V) { return static_cast<int32>(std::floor(V)); }
	inline int32 FloorToInt32(float V)  { return static_cast<int32>(std::floor(V)); }
	inline int32 RoundToInt32(double V) { return static_cast<int32>(std::round(V)); }
	inline int32 RoundToInt32(float V)  { return static_cast<int32>(std::round(V)); }
	inline float Square(float V) { return V * V; }
	inline double Square(double V) { return V * V; }
}

// Engine math constants used by MeshQueries.h and other code
#ifndef BIG_NUMBER
#define BIG_NUMBER (3.4e+38f)
#endif
#ifndef SMALL_NUMBER
#define SMALL_NUMBER (1.e-8f)
#endif
#ifndef KINDA_SMALL_NUMBER
#define KINDA_SMALL_NUMBER (1.e-4f)
#endif
#ifndef DOUBLE_BIG_NUMBER
#define DOUBLE_BIG_NUMBER (3.4e+38)
#endif
#ifndef UE_BIG_NUMBER
#define UE_BIG_NUMBER BIG_NUMBER
#endif

// FMathd / FMathf will be provided by the extracted MathUtil.h as typedefs of TMathUtil
// We cannot define them as namespaces here because MathUtil.h uses typedefs
// Instead, we provide standalone constants that code can use before MathUtil.h is included
namespace FMathdCompat
{
	constexpr double MaxReal = std::numeric_limits<double>::max();
	constexpr double Epsilon = 1e-7;
	constexpr double ZeroTolerance = 1e-4;
	constexpr double Pi = 3.14159265358979323846;
	constexpr double TwoPi = 2.0 * Pi;
	constexpr double HalfPi = Pi / 2.0;
	constexpr double InvSqrt3 = 1.0 / 1.7320508075688772;
	constexpr double DegToRad = Pi / 180.0;
	constexpr double RadToDeg = 180.0 / Pi;
}

// After MathUtil.h is included, FMathd and FMathf are typedefs of TMathUtil
// But our NavACD code uses FMathd::ZeroTolerance etc., so we need namespace-style access.
// We'll handle this by having NavACD.cpp include MathUtil.h first.

// ========================================================================
// Late-using declarations for types defined after the initial UE::Geometry block
// These must come after all the struct definitions above
// ========================================================================
namespace UE { namespace Geometry {
using ::FFrame3d;
// FTransformSRT3d intentionally not imported from global — use UE::Geometry's typedef.
using ::FTransform;
using ::FTransform3d;
// FMatrix3d is defined in UE::Geometry by MatrixTypes.h (typedef TMatrix3<double>)
// FPlane3d and FAxisAlignedBox3d are defined in PlaneTypes.h and BoxTypes.h
using ::FSphere;
using ::FHalfspace3d;
// FLine3d / FTriangle3d are defined in UE::Geometry by LineTypes.h / TriangleTypes.h
// (typedef'd from TLine3<double>/TTriangle3<double>); do not alias the global stubs.
using ::FSegment3d;
using ::FRay3d;
using ::FQuaterniond;
using ::FLinearColor;
using ::FString;
using ::FVector3d;
using ::FVector3f;
using ::FVector2d;
using ::FVector2f;
using ::FVector;
using ::FQuat4d;
using ::FQuat4f;
using ::FRotator;
}}

// Signal to extracted GeometryCore headers that our shim provides these types
// so they should skip their own typedefs to avoid redefinition conflicts
#define NAVACD_SHIM_TYPES_PROVIDED
