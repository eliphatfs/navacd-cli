// Copyright Epic Games, Inc. All Rights Reserved.
// NavACD standalone: stripped to UV/Normal/Color overlays only. Polygroup/Weight/
// Material/TriangleLabel subsystems are removed; stubs are kept for compile-time
// compatibility with any external callers.


#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/IndirectArray.h"
#include "Containers/Map.h"
#include "DynamicMesh/DynamicAttribute.h"
#include "DynamicMesh/DynamicMeshOverlay.h"
#include "GeometryTypes.h"
#include "HAL/PlatformCrt.h"
#include "InfoTypes.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector2D.h"
#include "Math/Vector4.h"
#include "Templates/UniquePtr.h"
#include "UObject/NameTypes.h"
#include "Util/DynamicVector.h"

class FArchive;
namespace DynamicMeshInfo { struct FEdgeCollapseInfo; }
namespace DynamicMeshInfo { struct FEdgeFlipInfo; }
namespace DynamicMeshInfo { struct FEdgeSplitInfo; }
namespace DynamicMeshInfo { struct FMergeEdgesInfo; }
namespace DynamicMeshInfo { struct FPokeTriangleInfo; }
namespace DynamicMeshInfo { struct FVertexSplitInfo; }

namespace UE
{
namespace Geometry
{
class FCompactMaps;
class FDynamicMesh3;

/** Standard UV overlay type - 2-element float */
typedef TDynamicMeshVectorOverlay<float, 2, FVector2f> FDynamicMeshUVOverlay;
/** Standard Normal overlay type - 3-element float */
typedef TDynamicMeshVectorOverlay<float, 3, FVector3f> FDynamicMeshNormalOverlay;
/** Standard Color overlay type - 4-element float (rbga) */
typedef TDynamicMeshVectorOverlay<float, 4, FVector4f> FDynamicMeshColorOverlay;

// NavACD standalone: polygroup/weight/material/triangle-label subsystems are
// stripped. Forward-declare incomplete types so external pointer-returning stub
// APIs still compile.
struct FDynamicMeshMaterialAttribute;
struct FDynamicMeshPolygroupAttribute;
struct FDynamicMeshWeightAttribute;
struct FDynamicMeshTriangleLabelAttribute;

/**
 * FDynamicMeshAttributeSet manages a set of extended attributes for a FDynamicMesh3.
 * NavACD standalone: UV/Normal/Color overlays only.
 */
class FDynamicMeshAttributeSet : public FDynamicMeshAttributeSetBase
{
public:
	GEOMETRYCORE_API FDynamicMeshAttributeSet(FDynamicMesh3* Mesh);

	GEOMETRYCORE_API FDynamicMeshAttributeSet(FDynamicMesh3* Mesh, int32 NumUVLayers, int32 NumNormalLayers);

	GEOMETRYCORE_API virtual ~FDynamicMeshAttributeSet() override;

	GEOMETRYCORE_API void Copy(const FDynamicMeshAttributeSet& Copy);

	/** returns true if the attached overlays/attributes are compact */
	GEOMETRYCORE_API bool IsCompact() const;

	GEOMETRYCORE_API void CompactCopy(const FCompactMaps& CompactMaps, const FDynamicMeshAttributeSet& Copy);

	GEOMETRYCORE_API void CompactInPlace(const FCompactMaps& CompactMaps);


	GEOMETRYCORE_API void SplitAllBowties(bool bParallel = true);


	GEOMETRYCORE_API void EnableMatchingAttributes(const FDynamicMeshAttributeSet& ToMatch, bool bClearExisting = true, bool bDiscardExtraAttributes = false);

	/** @return the parent mesh for this overlay */
	const FDynamicMesh3* GetParentMesh() const { return ParentMesh; }
	/** @return the parent mesh for this overlay */
	FDynamicMesh3* GetParentMesh() { return ParentMesh; }

private:
	/** @set the parent mesh for this overlay.  Only safe for use during FDynamicMesh move */
	GEOMETRYCORE_API void Reparent(FDynamicMesh3* NewParent);

public:

	/** @return true if the given edge is a seam edge in any overlay */
	GEOMETRYCORE_API virtual bool IsSeamEdge(int EdgeID) const;

	/** @return true if the given edge is the termination of a seam in any overlay*/
	GEOMETRYCORE_API virtual bool IsSeamEndEdge(int EdgeID) const;

	UE_DEPRECATED(5.3, "Please instead call the 4 argument version of IsSeamEdge, which distinguishes between tangent and normal seam edges.")
	GEOMETRYCORE_API virtual bool IsSeamEdge(int EdgeID, bool& bIsUVSeamOut, bool& bIsNormalSeamOut, bool& bIsColorSeamOut) const;

	GEOMETRYCORE_API virtual bool IsSeamEdge(int EdgeID, bool& bIsUVSeamOut, bool& bIsNormalSeamOut, bool& bIsColorSeamOut, bool& bIsTangentSeamOut) const;

	GEOMETRYCORE_API virtual bool IsSeamVertex(int VertexID, bool bBoundaryIsSeam = true) const;

	GEOMETRYCORE_API virtual bool IsSeamIntersectionVertex(int32 VertexID) const;

	/** @return false - material ID subsystem stripped in NavACD standalone */
	virtual bool IsMaterialBoundaryEdge(int EdgeID) const { return false; }

	//
	// UV Layers
	//

	virtual int NumUVLayers() const
	{
		return UVLayers.Num();
	}

	GEOMETRYCORE_API virtual void SetNumUVLayers(int Num);

	FDynamicMeshUVOverlay* GetUVLayer(int Index)
	{
		if (Index < UVLayers.Num() && Index > -1)
		{
			return &UVLayers[Index];
		}
		return nullptr;
	}

	const FDynamicMeshUVOverlay* GetUVLayer(int Index) const
	{
		if (Index < UVLayers.Num() && Index > -1)
		{
			return &UVLayers[Index];
		}
		return nullptr;
	}

	FDynamicMeshUVOverlay* PrimaryUV()
	{
		return GetUVLayer(0);
	}
	const FDynamicMeshUVOverlay* PrimaryUV() const
	{
		return GetUVLayer(0);
	}


	//
	// Normal Layers
	//

	virtual int NumNormalLayers() const
	{
		return NormalLayers.Num();
	}

	GEOMETRYCORE_API virtual void SetNumNormalLayers(int Num);

	GEOMETRYCORE_API void EnableTangents();

	GEOMETRYCORE_API void DisableTangents();

	FDynamicMeshNormalOverlay* GetNormalLayer(int Index)
	{
		if (Index < NormalLayers.Num() && Index > -1)
		{
			return &NormalLayers[Index];
		}
		return nullptr;
	}

	const FDynamicMeshNormalOverlay* GetNormalLayer(int Index) const
	{
		if (Index < NormalLayers.Num() && Index > -1)
		{
			return &NormalLayers[Index];
		}
		return nullptr;
	}


	FDynamicMeshNormalOverlay* PrimaryNormals()
	{
		return GetNormalLayer(0);
	}
	const FDynamicMeshNormalOverlay* PrimaryNormals() const
	{
		return GetNormalLayer(0);
	}
	FDynamicMeshNormalOverlay* PrimaryTangents()
	{
		return GetNormalLayer(1);
	}
	const FDynamicMeshNormalOverlay* PrimaryTangents() const
	{
		return GetNormalLayer(1);
	}
	FDynamicMeshNormalOverlay* PrimaryBiTangents()
	{
		return GetNormalLayer(2);
	}
	const FDynamicMeshNormalOverlay* PrimaryBiTangents() const
	{
		return GetNormalLayer(2);
	}

	bool HasTangentSpace() const
	{
		return (PrimaryNormals() != nullptr && PrimaryTangents()  != nullptr && PrimaryBiTangents() != nullptr);
	}

	bool HasPrimaryColors() const
	{
		return !!ColorLayer;
	}

	FDynamicMeshColorOverlay* PrimaryColors()
	{
		return ColorLayer.Get();
	}

	const FDynamicMeshColorOverlay* PrimaryColors() const
	{
		return ColorLayer.Get();
	}

	GEOMETRYCORE_API void EnablePrimaryColors();

	GEOMETRYCORE_API void DisablePrimaryColors();

	//
	// NavACD standalone: Polygroup/Weight/Material/TriangleLabel subsystems stripped.
	// Stubs preserved below to satisfy compilation of external call sites.
	//

	virtual int32 NumPolygroupLayers() const { return 0; }
	virtual void SetNumPolygroupLayers(int32 Num) {}
	FDynamicMeshPolygroupAttribute* GetPolygroupLayer(int Index) { return nullptr; }
	const FDynamicMeshPolygroupAttribute* GetPolygroupLayer(int Index) const { return nullptr; }

	virtual int32 NumWeightLayers() const { return 0; }
	virtual void SetNumWeightLayers(int32 Num) {}
	virtual void RemoveWeightLayer(int32 Index) {}
	FDynamicMeshWeightAttribute* GetWeightLayer(int Index) { return nullptr; }
	const FDynamicMeshWeightAttribute* GetWeightLayer(int Index) const { return nullptr; }
	void EnableMatchingWeightLayersByNames(const FDynamicMeshAttributeSet* ToMatch, bool bDiscardUnmatched) {}

	bool HasMaterialID() const { return false; }
	void EnableMaterialID() {}
	void DisableMaterialID() {}
	FDynamicMeshMaterialAttribute* GetMaterialID() { return nullptr; }
	const FDynamicMeshMaterialAttribute* GetMaterialID() const { return nullptr; }

	// Triangle label stubs
	void AttachTriangleLabelAttribute(FName InName, FDynamicMeshTriangleLabelAttribute* InAttribute) {}
	void RemoveTriangleLabelAttribute(FName InName) {}
	void RemoveAllTriangleLabelAttributes() {}
	bool HasTriangleLabelAttribute(FName InName) const { return false; }
	FDynamicMeshTriangleLabelAttribute* FindTriangleLabelAttribute(FName InName) const { return nullptr; }

	// NavACD standalone: sculpt layers stripped; provide a no-op count for callers
	inline int32 NumSculptLayers() const { return 0; }

	// Attach a new attribute (and transfer ownership of it to the attribute set)
	void AttachAttribute(FName AttribName, FDynamicMeshAttributeBase* Attribute)
	{
		if (GenericAttributes.Contains(AttribName))
		{
			UnregisterExternalAttribute(GenericAttributes[AttribName].Get());
		}
		GenericAttributes.Add(AttribName, TUniquePtr<FDynamicMeshAttributeBase>(Attribute));
		RegisterExternalAttribute(Attribute);
	}

	void RemoveAttribute(FName AttribName)
	{
		if (GenericAttributes.Contains(AttribName))
		{
			UnregisterExternalAttribute(GenericAttributes[AttribName].Get());
			GenericAttributes.Remove(AttribName);
		}
	}

	FDynamicMeshAttributeBase* GetAttachedAttribute(FName AttribName)
	{
		if (TUniquePtr<FDynamicMeshAttributeBase>* AttribPtrPtr = GenericAttributes.Find(AttribName))
		{
			return AttribPtrPtr->Get();
		}
		return nullptr;
	}

	const FDynamicMeshAttributeBase* GetAttachedAttribute(FName AttribName) const
	{
		if (const TUniquePtr<FDynamicMeshAttributeBase>* AttribPtrPtr = GenericAttributes.Find(AttribName))
		{
			return AttribPtrPtr->Get();
		}
		return nullptr;
	}
	int NumAttachedAttributes() const
	{
		return GenericAttributes.Num();
	}

	bool HasAttachedAttribute(FName AttribName) const
	{
		return GenericAttributes.Contains(AttribName);
	}

	const TMap<FName, TUniquePtr<FDynamicMeshAttributeBase>>& GetAttachedAttributes() const
	{
		return GenericAttributes;
	}

	GEOMETRYCORE_API bool IsSameAs(const FDynamicMeshAttributeSet& Other, bool bIgnoreDataLayout) const;

	GEOMETRYCORE_API SIZE_T GetByteCount() const;

protected:
	/** Parent mesh of this attribute set */
	FDynamicMesh3* ParentMesh;


	TIndirectArray<FDynamicMeshUVOverlay> UVLayers;
	TIndirectArray<FDynamicMeshNormalOverlay> NormalLayers;
	TUniquePtr<FDynamicMeshColorOverlay> ColorLayer;

	using GenericAttributesMap = TMap<FName, TUniquePtr<FDynamicMeshAttributeBase>>;
	GenericAttributesMap GenericAttributes;

protected:
	friend class FDynamicMesh3;

	void Initialize(int MaxVertexID, int MaxTriangleID)
	{
		for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
		{
			UVLayer.InitializeTriangles(MaxTriangleID);
		}
		for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
		{
			NormalLayer.InitializeTriangles(MaxTriangleID);
		}
	}

	GEOMETRYCORE_API virtual void OnNewTriangle(int TriangleID, bool bInserted);
	GEOMETRYCORE_API virtual void OnNewVertex(int VertexID, bool bInserted);
	GEOMETRYCORE_API virtual void OnRemoveTriangle(int TriangleID);
	GEOMETRYCORE_API virtual void OnRemoveVertex(int VertexID);
	GEOMETRYCORE_API virtual void OnReverseTriOrientation(int TriangleID);
	GEOMETRYCORE_API virtual void OnSplitEdge(const DynamicMeshInfo::FEdgeSplitInfo & splitInfo);
	GEOMETRYCORE_API virtual void OnFlipEdge(const DynamicMeshInfo::FEdgeFlipInfo & flipInfo);
	GEOMETRYCORE_API virtual void OnCollapseEdge(const DynamicMeshInfo::FEdgeCollapseInfo & collapseInfo);
	GEOMETRYCORE_API virtual void OnPokeTriangle(const DynamicMeshInfo::FPokeTriangleInfo & pokeInfo);
	GEOMETRYCORE_API virtual void OnMergeEdges(const DynamicMeshInfo::FMergeEdgesInfo & mergeInfo);
	GEOMETRYCORE_API virtual void OnMergeVertices(const DynamicMeshInfo::FMergeVerticesInfo& mergeInfo);
	GEOMETRYCORE_API virtual void OnSplitVertex(const DynamicMeshInfo::FVertexSplitInfo& SplitInfo, const TArrayView<const int>& TrianglesToUpdate);

	GEOMETRYCORE_API virtual bool CheckValidity(bool bAllowNonmanifold, EValidityCheckFailMode FailMode) const;

private:

	// Called by FDynamicMesh3 during a mesh append, to append a corresponding attribute set
	void Append(const FDynamicMeshAttributeSet& ToAppend, const FDynamicMesh3::FAppendInfo& AppendInfo);
	// Called by FDynamicMesh3 during a mesh append, to defaulted attributes when the ToAppend mesh did not have an attribute set
	void AppendDefaulted(const FDynamicMesh3::FAppendInfo& AppendInfo);
};



} // end namespace UE::Geometry
} // end namespace UE

