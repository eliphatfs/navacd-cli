// Copyright Epic Games, Inc. All Rights Reserved.
//
// NavACD standalone build: stripped down to UV/Normal/Color overlays only.
// Polygroup, weight, material ID, triangle label, skin-weight, morph-target,
// sculpt-layer, and serialization subsystems have been removed.

#include "DynamicMesh/DynamicMeshAttributeSet.h"

#include "IndexTypes.h"

using namespace UE::Geometry;


FDynamicMeshAttributeSet::FDynamicMeshAttributeSet(FDynamicMesh3* Mesh)
	: ParentMesh(Mesh)
{
	FDynamicMeshAttributeSet::SetNumUVLayers(1);
	FDynamicMeshAttributeSet::SetNumNormalLayers(1);
}

FDynamicMeshAttributeSet::FDynamicMeshAttributeSet(FDynamicMesh3* Mesh, int32 NumUVLayers, int32 NumNormalLayers)
	: ParentMesh(Mesh)
{
	FDynamicMeshAttributeSet::SetNumUVLayers(NumUVLayers);
	FDynamicMeshAttributeSet::SetNumNormalLayers(NumNormalLayers);
}

FDynamicMeshAttributeSet::~FDynamicMeshAttributeSet()
{
}

void FDynamicMeshAttributeSet::Copy(const FDynamicMeshAttributeSet& Copy)
{
	SetNumUVLayers(Copy.NumUVLayers());
	for (int UVIdx = 0; UVIdx < NumUVLayers(); UVIdx++)
	{
		UVLayers[UVIdx].Copy(Copy.UVLayers[UVIdx]);
	}
	SetNumNormalLayers(Copy.NumNormalLayers());
	for (int NormalLayerIndex = 0; NormalLayerIndex < NumNormalLayers(); NormalLayerIndex++)
	{
		NormalLayers[NormalLayerIndex].Copy(Copy.NormalLayers[NormalLayerIndex]);
	}
	if (Copy.ColorLayer)
	{
		EnablePrimaryColors();
		ColorLayer->Copy(*(Copy.ColorLayer));
	}
	else
	{
		DisablePrimaryColors();
	}

	ResetRegisteredAttributes();

	GenericAttributes.Reset();
	for (const TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : Copy.GenericAttributes)
	{
		AttachAttribute(AttribPair.Key, AttribPair.Value->MakeCopy(ParentMesh));
	}

	// parent mesh is *not* copied!
}


bool FDynamicMeshAttributeSet::IsCompact() const
{
	for (int UVIdx = 0; UVIdx < NumUVLayers(); UVIdx++)
	{
		if (!UVLayers[UVIdx].IsCompact())
		{
			return false;
		}
	}
	for (int NormalLayerIndex = 0; NormalLayerIndex < NumNormalLayers(); NormalLayerIndex++)
	{
		if (!NormalLayers[NormalLayerIndex].IsCompact())
		{
			return false;
		}
	}
	if (HasPrimaryColors())
	{
		if (!ColorLayer->IsCompact())
		{
			return false;
		}
	}
	return true;
}



void FDynamicMeshAttributeSet::CompactCopy(const FCompactMaps& CompactMaps, const FDynamicMeshAttributeSet& Copy)
{
	SetNumUVLayers(Copy.NumUVLayers());
	for (int UVIdx = 0; UVIdx < NumUVLayers(); UVIdx++)
	{
		UVLayers[UVIdx].CompactCopy(CompactMaps, Copy.UVLayers[UVIdx]);
	}
	SetNumNormalLayers(Copy.NumNormalLayers());
	for (int NormalLayerIndex = 0; NormalLayerIndex < NumNormalLayers(); NormalLayerIndex++)
	{
		NormalLayers[NormalLayerIndex].CompactCopy(CompactMaps, Copy.NormalLayers[NormalLayerIndex]);
	}
	if (Copy.ColorLayer)
	{
		EnablePrimaryColors();
		ColorLayer->CompactCopy(CompactMaps, *(Copy.ColorLayer));
	}
	else
	{
		DisablePrimaryColors();
	}

	ResetRegisteredAttributes();

	GenericAttributes.Reset();
	for (const TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : Copy.GenericAttributes)
	{
		AttachAttribute(AttribPair.Key, AttribPair.Value->MakeCompactCopy(CompactMaps, ParentMesh));
	}
}


void FDynamicMeshAttributeSet::Append(const FDynamicMeshAttributeSet& ToAppend, const FDynamicMesh3::FAppendInfo& AppendInfo)
{
	auto AppendHelper = [&AppendInfo]<typename T>(T & Target, const T * ToAppendPtr)
	{
		if (ToAppendPtr)
		{
			Target.Append(*ToAppendPtr, AppendInfo);
		}
		else
		{
			Target.AppendDefaulted(AppendInfo);
		}
	};

	for (int32 Idx = 0; Idx < NumUVLayers(); ++Idx)
	{
		AppendHelper(UVLayers[Idx], ToAppend.GetUVLayer(Idx));
	}
	for (int32 Idx = 0; Idx < NumNormalLayers(); ++Idx)
	{
		AppendHelper(NormalLayers[Idx], ToAppend.GetNormalLayer(Idx));
	}
	if (ColorLayer.IsValid())
	{
		AppendHelper(*ColorLayer, ToAppend.ColorLayer.Get());
	}

	for (TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : GenericAttributes)
	{
		const TUniquePtr<FDynamicMeshAttributeBase>* AppendAttr = ToAppend.GenericAttributes.Find(AttribPair.Key);
		AppendHelper(*AttribPair.Value, AppendAttr ? AppendAttr->Get() : nullptr);
	}
}

void FDynamicMeshAttributeSet::AppendDefaulted(const FDynamicMesh3::FAppendInfo& AppendInfo)
{
	auto AppendHelper = [&AppendInfo]<typename T>(T & Target)
	{
		Target.AppendDefaulted(AppendInfo);
	};

	for (int32 Idx = 0; Idx < NumUVLayers(); ++Idx)
	{
		AppendHelper(UVLayers[Idx]);
	}
	for (int32 Idx = 0; Idx < NumNormalLayers(); ++Idx)
	{
		AppendHelper(NormalLayers[Idx]);
	}
	if (ColorLayer.IsValid())
	{
		AppendHelper(*ColorLayer);
	}

	for (const TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : GenericAttributes)
	{
		AppendHelper(*AttribPair.Value);
	}
}



void FDynamicMeshAttributeSet::CompactInPlace(const FCompactMaps& CompactMaps)
{
	for (int UVIdx = 0; UVIdx < NumUVLayers(); UVIdx++)
	{
		UVLayers[UVIdx].CompactInPlace(CompactMaps);
	}
	for (int NormalLayerIndex = 0; NormalLayerIndex < NumNormalLayers(); NormalLayerIndex++)
	{
		NormalLayers[NormalLayerIndex].CompactInPlace(CompactMaps);
	}
	if (ColorLayer.IsValid())
	{
		ColorLayer->CompactInPlace(CompactMaps);
	}

	for (FDynamicMeshAttributeBase* RegAttrib : RegisteredAttributes)
	{
		RegAttrib->CompactInPlace(CompactMaps);
	}
}



void FDynamicMeshAttributeSet::SplitAllBowties(bool bParallel)
{
	int32 UVLayerCount = NumUVLayers();
	int32 NormalLayerCount = NumNormalLayers();

	auto SplitOne = [](auto Overlay)->void
	{
		Overlay->SplitBowties();
	};

	for (int32 i = 0; i < UVLayerCount; ++i)
	{
		FDynamicMeshUVOverlay* UVLayer = GetUVLayer(i);
		SplitOne(UVLayer);
	}
	for (int32 i = 0; i < NormalLayerCount; ++i)
	{
		FDynamicMeshNormalOverlay* NormalLayer = GetNormalLayer(i);
		SplitOne(NormalLayer);
	}
	if (HasPrimaryColors())
	{
		FDynamicMeshColorOverlay* Colors = PrimaryColors();
		SplitOne(Colors);
	}
}



void FDynamicMeshAttributeSet::EnableMatchingAttributes(const FDynamicMeshAttributeSet& ToMatch, bool bClearExisting, bool bDiscardExtraAttributes)
{
	int32 ExistingUVLayers = NumUVLayers();
	int32 RequiredUVLayers = (bClearExisting || bDiscardExtraAttributes) ? ToMatch.NumUVLayers() : FMath::Max(ExistingUVLayers, ToMatch.NumUVLayers());
	SetNumUVLayers(RequiredUVLayers);
	for (int32 k = bClearExisting ? 0 : ExistingUVLayers; k < NumUVLayers(); k++)
	{
		UVLayers[k].ClearElements();
	}

	int32 ExistingNormalLayers = NumNormalLayers();
	int32 RequiredNormalLayers = (bClearExisting || bDiscardExtraAttributes) ? ToMatch.NumNormalLayers() : FMath::Max(ExistingNormalLayers, ToMatch.NumNormalLayers());
	SetNumNormalLayers(RequiredNormalLayers);
	for (int32 k = bClearExisting ? 0 : ExistingNormalLayers; k < NumNormalLayers(); k++)
	{
		NormalLayers[k].ClearElements();
	}

	bool bWantColorLayer = (bClearExisting || bDiscardExtraAttributes) ? ToMatch.HasPrimaryColors() : ( ToMatch.HasPrimaryColors() || this->HasPrimaryColors() );
	if (bClearExisting || bWantColorLayer == false)
	{
		DisablePrimaryColors();
	}
	if (bWantColorLayer)
	{
		EnablePrimaryColors();
	}

	if (bClearExisting)
	{
		GenericAttributes.Reset();
		ResetRegisteredAttributes();

		for (const TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : ToMatch.GenericAttributes)
		{
			AttachAttribute(AttribPair.Key, AttribPair.Value->MakeNew(ParentMesh));
		}
	}
	else
	{
		if (bDiscardExtraAttributes)
		{
			GenericAttributesMap ExistingGenericAttributes = MoveTemp(GenericAttributes);
			GenericAttributes.Reset();

			ResetRegisteredAttributes();
			for (TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : ExistingGenericAttributes)
			{
				if ( ToMatch.GenericAttributes.Contains(AttribPair.Key) )
				{
					AttachAttribute(AttribPair.Key, AttribPair.Value.Release());
				}
			}
		}

		for (const TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : ToMatch.GenericAttributes)
		{
			TUniquePtr<FDynamicMeshAttributeBase>* FoundMatch = GenericAttributes.Find(AttribPair.Key);
			if (FoundMatch == nullptr)
			{
				AttachAttribute(AttribPair.Key, AttribPair.Value->MakeNew(ParentMesh));
			}
		}
	}
}



void FDynamicMeshAttributeSet::Reparent(FDynamicMesh3* NewParent)
{
	ParentMesh = NewParent;

	for (int UVIdx = 0; UVIdx < NumUVLayers(); UVIdx++)
	{
		UVLayers[UVIdx].Reparent(NewParent);
	}
	for (int NormalLayerIndex = 0; NormalLayerIndex < NumNormalLayers(); NormalLayerIndex++)
	{
		NormalLayers[NormalLayerIndex].Reparent(NewParent);
	}
	if (ColorLayer)
	{
		ColorLayer->Reparent(NewParent);
	}

	for (FDynamicMeshAttributeBase* RegAttrib : RegisteredAttributes)
	{
		RegAttrib->Reparent(NewParent);
	}
}



void FDynamicMeshAttributeSet::SetNumUVLayers(int Num)
{
	if (UVLayers.Num() == Num)
	{
		return;
	}
	if (Num >= UVLayers.Num())
	{
		for (int i = (int)UVLayers.Num(); i < Num; ++i)
		{
			FDynamicMeshUVOverlay* NewUVLayer = new FDynamicMeshUVOverlay(ParentMesh);
			NewUVLayer->InitializeTriangles(ParentMesh->MaxTriangleID());
			UVLayers.Add(NewUVLayer);
		}
	}
	else
	{
		UVLayers.RemoveAt(Num, UVLayers.Num() - Num);
	}
	ensure(UVLayers.Num() == Num);
}



void FDynamicMeshAttributeSet::EnableTangents()
{
	SetNumNormalLayers(3);
}

void FDynamicMeshAttributeSet::DisableTangents()
{
	SetNumNormalLayers(1);
}


void FDynamicMeshAttributeSet::SetNumNormalLayers(int Num)
{
	if (NormalLayers.Num() == Num)
	{
		return;
	}
	if (Num >= NormalLayers.Num())
	{
		for (int32 i = NormalLayers.Num(); i < Num; ++i)
		{
			FDynamicMeshNormalOverlay* NewNormalLayer = new FDynamicMeshNormalOverlay(ParentMesh);
			NewNormalLayer->InitializeTriangles(ParentMesh->MaxTriangleID());
			NormalLayers.Add(NewNormalLayer);
		}
	}
	else
	{
		NormalLayers.RemoveAt(Num, NormalLayers.Num() - Num);
	}
	ensure(NormalLayers.Num() == Num);
}

void FDynamicMeshAttributeSet::EnablePrimaryColors()
{
	if (HasPrimaryColors() == false)
	{
		ColorLayer = MakeUnique<FDynamicMeshColorOverlay>(ParentMesh);
		ColorLayer->InitializeTriangles(ParentMesh->MaxTriangleID());
	}
}

void FDynamicMeshAttributeSet::DisablePrimaryColors()
{
	ColorLayer.Reset();
}


bool FDynamicMeshAttributeSet::IsSeamEdge(int eid) const
{
	for (const FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		if (UVLayer.IsSeamEdge(eid))
		{
			return true;
		}
	}

	for (const FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		if (NormalLayer.IsSeamEdge(eid))
		{
			return true;
		}
	}

	if (ColorLayer && ColorLayer->IsSeamEdge(eid))
	{
		return true;
	}

	return false;
}

bool FDynamicMeshAttributeSet::IsSeamEndEdge(int eid) const
{
	for (const FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		if (UVLayer.IsSeamEndEdge(eid))
		{
			return true;
		}
	}

	for (const FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		if (NormalLayer.IsSeamEndEdge(eid))
		{
			return true;
		}
	}

	if (ColorLayer && ColorLayer->IsSeamEndEdge(eid))
	{
		return true;
	}
	return false;
}

bool FDynamicMeshAttributeSet::IsSeamEdge(int EdgeID, bool& bIsUVSeamOut, bool& bIsNormalSeamOut, bool& bIsColorSeamOut) const
{
	bool bIsTangentSeam;
	bool IsSeam = IsSeamEdge(EdgeID, bIsUVSeamOut, bIsNormalSeamOut, bIsColorSeamOut, bIsTangentSeam);
	bIsNormalSeamOut = bIsNormalSeamOut || bIsTangentSeam;
	return IsSeam;
}

bool FDynamicMeshAttributeSet::IsSeamEdge(int EdgeID, bool& bIsUVSeamOut, bool& bIsNormalSeamOut, bool& bIsColorSeamOut, bool& bIsTangentSeamOut) const
{
	bIsUVSeamOut = false;
	for (const FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		if (UVLayer.IsSeamEdge(EdgeID))
		{
			bIsUVSeamOut = true;
		}
	}

	bIsNormalSeamOut = !NormalLayers.IsEmpty() && NormalLayers[0].IsSeamEdge(EdgeID);
	bIsTangentSeamOut = false;
	for (int32 LayerIdx = 1; LayerIdx < NormalLayers.Num(); ++LayerIdx)
	{
		const FDynamicMeshNormalOverlay& NormalLayer = NormalLayers[LayerIdx];
		if (NormalLayer.IsSeamEdge(EdgeID))
		{
			bIsTangentSeamOut = true;
		}
	}

	bIsColorSeamOut = false;
	if (ColorLayer && ColorLayer->IsSeamEdge(EdgeID))
	{
		bIsColorSeamOut = true;
	}
	return (bIsUVSeamOut || bIsNormalSeamOut || bIsColorSeamOut || bIsTangentSeamOut);
}


bool FDynamicMeshAttributeSet::IsSeamVertex(int VID, bool bBoundaryIsSeam) const
{
	for (const FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		if (UVLayer.IsSeamVertex(VID, bBoundaryIsSeam))
		{
			return true;
		}
	}
	for (const FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		if (NormalLayer.IsSeamVertex(VID, bBoundaryIsSeam))
		{
			return true;
		}
	}
	if (ColorLayer && ColorLayer->IsSeamVertex(VID, bBoundaryIsSeam))
	{
		return true;
	}
	return false;
}

bool FDynamicMeshAttributeSet::IsSeamIntersectionVertex(int32 VertexID) const
{
	for (const FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		if (UVLayer.IsSeamIntersectionVertex(VertexID))
		{
			return true;
		}
	}
	for (const FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		if (NormalLayer.IsSeamIntersectionVertex(VertexID))
		{
			return true;
		}
	}
	if (ColorLayer && ColorLayer->IsSeamIntersectionVertex(VertexID))
	{
		return true;
	}
	return false;
}

void FDynamicMeshAttributeSet::OnNewVertex(int VertexID, bool bInserted)
{
	FDynamicMeshAttributeSetBase::OnNewVertex(VertexID, bInserted);

	for (TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : GenericAttributes)
	{
		AttribPair.Value->OnNewVertex(VertexID, bInserted);
	}
}


void FDynamicMeshAttributeSet::OnRemoveVertex(int VertexID)
{
	FDynamicMeshAttributeSetBase::OnRemoveVertex(VertexID);

	for (TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& AttribPair : GenericAttributes)
	{
		AttribPair.Value->OnRemoveVertex(VertexID);
	}
}


void FDynamicMeshAttributeSet::OnNewTriangle(int TriangleID, bool bInserted)
{
	FDynamicMeshAttributeSetBase::OnNewTriangle(TriangleID, bInserted);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.InitializeNewTriangle(TriangleID);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.InitializeNewTriangle(TriangleID);
	}
	if (ColorLayer)
	{
		ColorLayer->InitializeNewTriangle(TriangleID);
	}
}


void FDynamicMeshAttributeSet::OnRemoveTriangle(int TriangleID)
{
	FDynamicMeshAttributeSetBase::OnRemoveTriangle(TriangleID);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnRemoveTriangle(TriangleID);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnRemoveTriangle(TriangleID);
	}
	if (ColorLayer)
	{
		ColorLayer->OnRemoveTriangle(TriangleID);
	}
}

void FDynamicMeshAttributeSet::OnReverseTriOrientation(int TriangleID)
{
	FDynamicMeshAttributeSetBase::OnReverseTriOrientation(TriangleID);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnReverseTriOrientation(TriangleID);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnReverseTriOrientation(TriangleID);
	}
	if (ColorLayer)
	{
		ColorLayer->OnReverseTriOrientation(TriangleID);
	}
}

void FDynamicMeshAttributeSet::OnSplitEdge(const FDynamicMesh3::FEdgeSplitInfo& SplitInfo)
{
	FDynamicMeshAttributeSetBase::OnSplitEdge(SplitInfo);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnSplitEdge(SplitInfo);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnSplitEdge(SplitInfo);
	}
	if (ColorLayer)
	{
		ColorLayer->OnSplitEdge(SplitInfo);
	}
}

void FDynamicMeshAttributeSet::OnFlipEdge(const FDynamicMesh3::FEdgeFlipInfo & flipInfo)
{
	FDynamicMeshAttributeSetBase::OnFlipEdge(flipInfo);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnFlipEdge(flipInfo);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnFlipEdge(flipInfo);
	}
	if (ColorLayer)
	{
		ColorLayer->OnFlipEdge(flipInfo);
	}
}


void FDynamicMeshAttributeSet::OnCollapseEdge(const FDynamicMesh3::FEdgeCollapseInfo & collapseInfo)
{
	FDynamicMeshAttributeSetBase::OnCollapseEdge(collapseInfo);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnCollapseEdge(collapseInfo);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnCollapseEdge(collapseInfo);
	}
	if (ColorLayer)
	{
		ColorLayer->OnCollapseEdge(collapseInfo);
	}
}

void FDynamicMeshAttributeSet::OnPokeTriangle(const FDynamicMesh3::FPokeTriangleInfo & pokeInfo)
{
	FDynamicMeshAttributeSetBase::OnPokeTriangle(pokeInfo);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnPokeTriangle(pokeInfo);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnPokeTriangle(pokeInfo);
	}
	if (ColorLayer)
	{
		ColorLayer->OnPokeTriangle(pokeInfo);
	}
}

void FDynamicMeshAttributeSet::OnMergeEdges(const FDynamicMesh3::FMergeEdgesInfo & mergeInfo)
{
	FDynamicMeshAttributeSetBase::OnMergeEdges(mergeInfo);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnMergeEdges(mergeInfo);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnMergeEdges(mergeInfo);
	}
	if (ColorLayer)
	{
		ColorLayer->OnMergeEdges(mergeInfo);
	}
}

void FDynamicMeshAttributeSet::OnMergeVertices(const DynamicMeshInfo::FMergeVerticesInfo& MergeInfo)
{
	FDynamicMeshAttributeSetBase::OnMergeVertices(MergeInfo);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnMergeVertices(MergeInfo);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnMergeVertices(MergeInfo);
	}
	if (ColorLayer)
	{
		ColorLayer->OnMergeVertices(MergeInfo);
	}
}

void FDynamicMeshAttributeSet::OnSplitVertex(const DynamicMeshInfo::FVertexSplitInfo& SplitInfo, const TArrayView<const int>& TrianglesToUpdate)
{
	FDynamicMeshAttributeSetBase::OnSplitVertex(SplitInfo, TrianglesToUpdate);

	for (FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		UVLayer.OnSplitVertex(SplitInfo, TrianglesToUpdate);
	}
	for (FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		NormalLayer.OnSplitVertex(SplitInfo, TrianglesToUpdate);
	}
	if (ColorLayer)
	{
		ColorLayer->OnSplitVertex(SplitInfo, TrianglesToUpdate);
	}
}

bool FDynamicMeshAttributeSet::IsSameAs(const FDynamicMeshAttributeSet& Other, bool bIgnoreDataLayout) const
{
	if (UVLayers.Num() != Other.UVLayers.Num() ||
		NormalLayers.Num() != Other.NormalLayers.Num())
	{
		return false;
	}

	for (int Idx = 0; Idx < UVLayers.Num(); Idx++)
	{
		if (!UVLayers[Idx].IsSameAs(Other.UVLayers[Idx], bIgnoreDataLayout))
		{
			return false;
		}
	}

	for (int Idx = 0; Idx < NormalLayers.Num(); Idx++)
	{
		if (!NormalLayers[Idx].IsSameAs(Other.NormalLayers[Idx], bIgnoreDataLayout))
		{
			return false;
		}
	}

	if (HasPrimaryColors() != Other.HasPrimaryColors())
	{
		return false;
	}
	if (HasPrimaryColors())
	{
		if (!ColorLayer->IsSameAs(*Other.ColorLayer, bIgnoreDataLayout))
		{
			return false;
		}
	}

	return true;
}


SIZE_T FDynamicMeshAttributeSet::GetByteCount() const
{
	SIZE_T ByteCount = 0;

	for (const FDynamicMeshUVOverlay& UVLayer : UVLayers)
	{
		ByteCount += UVLayer.GetByteCount();
	}

	for (const FDynamicMeshNormalOverlay& NormalLayer : NormalLayers)
	{
		ByteCount += NormalLayer.GetByteCount();
	}

	if (ColorLayer)
	{
		ByteCount += ColorLayer->GetByteCount();
	}

	for (const TPair<FName, TUniquePtr<FDynamicMeshAttributeBase>>& GenericAttribute : GenericAttributes)
	{
		ByteCount += GenericAttribute.Value->GetByteCount();
	}

	return ByteCount;
}


bool FDynamicMeshAttributeSet::CheckValidity(bool bAllowNonmanifold, EValidityCheckFailMode FailMode) const
{
	bool bValid = FDynamicMeshAttributeSetBase::CheckValidity(bAllowNonmanifold, FailMode);
	for (int UVLayerIndex = 0; UVLayerIndex < NumUVLayers(); UVLayerIndex++)
	{
		bValid = GetUVLayer(UVLayerIndex)->CheckValidity(bAllowNonmanifold, FailMode) && bValid;
	}
	bValid = PrimaryNormals()->CheckValidity(bAllowNonmanifold, FailMode) && bValid;
	if (ColorLayer)
	{
		bValid = ColorLayer->CheckValidity(bAllowNonmanifold, FailMode) && bValid;
	}
	return bValid;
}
