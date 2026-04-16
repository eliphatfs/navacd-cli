// Copyright Epic Games, Inc. All Rights Reserved.
//
// NavACD standalone build: minimal subset of MeshTransforms. Only implements
// the entry points called by our pipeline (ApplyTransform with an FTransformSRT3d).
// Sculpt-layer, ParallelFor, and full attribute-transform variants are stubbed.

#include "DynamicMesh/MeshTransforms.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "DynamicMesh/DynamicMeshOverlay.h"

using namespace UE::Geometry;

namespace MeshTransformsLocal
{
	static inline FVector3d TransformPos(const FTransformSRT3d& T, const FVector3d& P)
	{
		return T.TransformPosition(P);
	}
	static inline FVector3f TransformNormal(const FTransformSRT3d& T, const FVector3f& N)
	{
		// Normal transform: use inverse transpose of linear part.  For uniform scale / rotation,
		// simply rotating by the transform's rotation is a good approximation.  NavACD
		// standalone only uses positions, so a simple rotation suffices here.
		FVector3d Nd((double)N.X, (double)N.Y, (double)N.Z);
		FVector3d Rotated = T.TransformVector(Nd);
		Rotated.Normalize();
		return FVector3f((float)Rotated.X, (float)Rotated.Y, (float)Rotated.Z);
	}
}

void MeshTransforms::ApplyTransform(FDynamicMesh3& Mesh, const FTransformSRT3d& Transform, bool bReverseOrientationIfNeeded, ETransformAttributes TransformAttributes)
{
	const int32 MaxVID = Mesh.MaxVertexID();
	for (int32 vid = 0; vid < MaxVID; ++vid)
	{
		if (Mesh.IsVertex(vid))
		{
			FVector3d P = Mesh.GetVertex(vid);
			Mesh.SetVertex(vid, MeshTransformsLocal::TransformPos(Transform, P));

			if (Mesh.HasVertexNormals())
			{
				FVector3f N = Mesh.GetVertexNormal(vid);
				Mesh.SetVertexNormal(vid, MeshTransformsLocal::TransformNormal(Transform, N));
			}
		}
	}

	if (Mesh.HasAttributes())
	{
		FDynamicMeshAttributeSet* Attr = Mesh.Attributes();
		const int NumNormalLayers = Attr->NumNormalLayers();
		for (int NL = 0; NL < NumNormalLayers; ++NL)
		{
			FDynamicMeshNormalOverlay* NormalLayer = Attr->GetNormalLayer(NL);
			if (!NormalLayer) continue;
			const int32 MaxEID = NormalLayer->MaxElementID();
			for (int32 eid = 0; eid < MaxEID; ++eid)
			{
				if (NormalLayer->IsElement(eid))
				{
					FVector3f N = NormalLayer->GetElement(eid);
					NormalLayer->SetElement(eid, MeshTransformsLocal::TransformNormal(Transform, N));
				}
			}
		}
	}

	// Note: bReverseOrientationIfNeeded is a hint NavACD does not need in practice; treated as no-op here.
	(void)bReverseOrientationIfNeeded;
	(void)TransformAttributes;
}

void MeshTransforms::ApplyTransformInverse(FDynamicMesh3& Mesh, const FTransformSRT3d& Transform, bool bReverseOrientationIfNeeded, ETransformAttributes TransformAttributes)
{
	const int32 MaxVID = Mesh.MaxVertexID();
	for (int32 vid = 0; vid < MaxVID; ++vid)
	{
		if (Mesh.IsVertex(vid))
		{
			FVector3d P = Mesh.GetVertex(vid);
			Mesh.SetVertex(vid, Transform.InverseTransformPosition(P));
		}
	}
	(void)bReverseOrientationIfNeeded;
	(void)TransformAttributes;
}

void MeshTransforms::Translate(FDynamicMesh3& Mesh, const FVector3d& Translation, ETransformAttributes TransformAttributes)
{
	FTransformSRT3d T(Translation);
	ApplyTransform(Mesh, T, false, TransformAttributes);
}

void MeshTransforms::Scale(FDynamicMesh3& Mesh, const FVector3d& Scale, const FVector3d& Origin, bool bReverseOrientationIfNeeded, bool bOnlyPositions)
{
	const int32 MaxVID = Mesh.MaxVertexID();
	for (int32 vid = 0; vid < MaxVID; ++vid)
	{
		if (Mesh.IsVertex(vid))
		{
			FVector3d P = Mesh.GetVertex(vid);
			FVector3d NP((P.X - Origin.X) * Scale.X + Origin.X,
			             (P.Y - Origin.Y) * Scale.Y + Origin.Y,
			             (P.Z - Origin.Z) * Scale.Z + Origin.Z);
			Mesh.SetVertex(vid, NP);
		}
	}
	(void)bReverseOrientationIfNeeded;
	(void)bOnlyPositions;
}

void MeshTransforms::Scale(FDynamicMesh3& Mesh, const FVector3d& Scale, const FVector3d& Origin, bool bReverseOrientationIfNeeded, ETransformAttributes TransformAttributes)
{
	MeshTransforms::Scale(Mesh, Scale, Origin, bReverseOrientationIfNeeded, false);
	(void)TransformAttributes;
}

void MeshTransforms::Rotate(FDynamicMesh3& Mesh, const FRotator& Rotation, const FVector3d& RotationOrigin, ETransformAttributes TransformAttributes)
{
	// NavACD standalone: rotation not used on the convex-decomposition path; no-op
	(void)Mesh; (void)Rotation; (void)RotationOrigin; (void)TransformAttributes;
}

void MeshTransforms::WorldToFrameCoords(FDynamicMesh3& Mesh, const FFrame3d& Frame, ETransformAttributes TransformAttributes)
{
	(void)Mesh; (void)Frame; (void)TransformAttributes;
}

void MeshTransforms::FrameCoordsToWorld(FDynamicMesh3& Mesh, const FFrame3d& Frame, ETransformAttributes TransformAttributes)
{
	(void)Mesh; (void)Frame; (void)TransformAttributes;
}

void MeshTransforms::ApplyTransform(FDynamicMesh3& Mesh,
	TFunctionRef<FVector3d(const FVector3d&)> PositionTransform,
	TFunctionRef<FVector3f(const FVector3f&)> NormalTransform)
{
	const int32 MaxVID = Mesh.MaxVertexID();
	for (int32 vid = 0; vid < MaxVID; ++vid)
	{
		if (Mesh.IsVertex(vid))
		{
			Mesh.SetVertex(vid, PositionTransform(Mesh.GetVertex(vid)));
		}
	}
	(void)NormalTransform;
}

void MeshTransforms::ApplyTransform(FDynamicMesh3& Mesh,
	TFunctionRef<FVector3d(const FVector3d&)> PositionTransform,
	TFunctionRef<FVector3f(const FVector3f&)> NormalTransform,
	TFunctionRef<FVector3f(const FVector3f&)> TangentTransform,
	ETransformAttributes TransformAttributes)
{
	MeshTransforms::ApplyTransform(Mesh, PositionTransform, NormalTransform);
	(void)TangentTransform;
	(void)TransformAttributes;
}

void MeshTransforms::ReverseOrientationIfNeeded(FDynamicMesh3& Mesh, const FTransformSRT3d& Transform)
{
	(void)Mesh; (void)Transform;
}
