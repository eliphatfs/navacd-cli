#include "NavACD.h"
#include "MeshQueries.h"
#include "Util/ColorConstants.h"

#include <fstream>
#include <iomanip>

using namespace UE::Geometry;

namespace NavACD
{

FConvexDecomposition3 Run(
	const FDynamicMesh3& Mesh,
	double MinRadiusFrac,
	double ToleranceFrac,
	bool bIgnoreUnreachableInternalSpace,
	TArrayView<const FVector3d> CustomNavigablePositions)
{
	FAxisAlignedBox3d Bounds = Mesh.GetBounds();
	double MaxDim = Bounds.MaxDim();
	double UseMinRadius = MinRadiusFrac * MaxDim;
	double UseTolerance = ToleranceFrac * MaxDim;

	FConvexDecomposition3::FPreprocessMeshOptions PreprocessOptions;
	PreprocessOptions.bMergeEdges = true;
	PreprocessOptions.CustomPreprocess = [](FDynamicMesh3& ProcessMesh, const FAxisAlignedBox3d& Bounds) -> void
	{
		// for solid inputs, flip orientation if the initial volume is negative
		if (ProcessMesh.IsClosed())
		{
			double InitialVolume = TMeshQueries<FDynamicMesh3>::GetVolumeArea(ProcessMesh).X;
			if (InitialVolume < 0)
			{
				ProcessMesh.ReverseOrientation();
			}
		}
	};

	FConvexDecomposition3 ConvexDecomposition(Mesh, PreprocessOptions);
	const bool bIsSolid = ConvexDecomposition.IsInputSolid();
	ConvexDecomposition.bTreatAsSolid = bIsSolid;

	FNegativeSpaceSampleSettings NegativeSpaceSettings;
	NegativeSpaceSettings.ApplyDefaults();
	NegativeSpaceSettings.SampleMethod = FNegativeSpaceSampleSettings::ESampleMethod::NavigableVoxelSearch;
	NegativeSpaceSettings.bDeterministic = true;
	NegativeSpaceSettings.bRequireSearchSampleCoverage = true;
	NegativeSpaceSettings.bOnlyConnectedToHull = bIgnoreUnreachableInternalSpace;
	NegativeSpaceSettings.TargetNumSamples = 0;
	NegativeSpaceSettings.bAllowSamplesInsideMesh = !bIsSolid;

	NegativeSpaceSettings.ReduceRadiusMargin = UseTolerance;
	NegativeSpaceSettings.MinRadius = UseMinRadius;
	NegativeSpaceSettings.MinSpacing = 0;

	ConvexDecomposition.InitializeNegativeSpace(NegativeSpaceSettings, CustomNavigablePositions);

	ConvexDecomposition.MaxConvexEdgePlanes = 4;
	ConvexDecomposition.bSplitDisconnectedComponents = false;
	ConvexDecomposition.ConvexEdgeAngleMoreSamplesThreshold = 180; // a high value to disable this feature
	ConvexDecomposition.ThickenAfterHullFailure = FMath::Max(FMathd::ZeroTolerance, NegativeSpaceSettings.ReduceRadiusMargin * .01);
	constexpr int32 MaxAllowedSplits = 1000000; // more parts than any expected / reasonable decomposition
	for (int32 Split = 0; ; Split++)
	{
		int32 NumSplit = ConvexDecomposition.SplitWorst(false, -1, true, NegativeSpaceSettings.ReduceRadiusMargin * .5);

		if (NumSplit == 0)
		{
			break;
		}

		if (!ensureMsgf(Split < MaxAllowedSplits, "Convex decomposition split the input %d times; likely stuck in a loop", Split))
		{
			break;
		}
	}

	ConvexDecomposition.FixHullOverlapsInNegativeSpace();

	int32 NumHullsBefore = ConvexDecomposition.NumHulls();
	constexpr double MinThicknessToleranceWorldSpace = 0;
	int32 NumMerged = ConvexDecomposition.MergeBest(-1, 0, MinThicknessToleranceWorldSpace, true);

	return ConvexDecomposition;
}

} // namespace NavACD
