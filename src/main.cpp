#include "NavACD.h"
#include "CompGeom/ConvexDecomposition3.h"
#include "CompGeom/ExactPredicates.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Util/ColorConstants.h"
#include "IO/OBJMeshUtil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <limits>

using namespace UE::Geometry;

// Simple command-line argument parser
struct FArgs
{
	std::string InputPath;
	std::string OutputPath;
	double MinRadiusFrac = 0;
	double ToleranceFrac = 0;
	bool bStats = false;
	bool bProtectUnreachable = false;
	bool bHelp = false;
	bool bHasMinRadius = false;
	bool bHasTolerance = false;
};

static FArgs ParseArgs(int ArgC, char* ArgV[])
{
	FArgs Args;
	for (int i = 1; i < ArgC; i++)
	{
		if (std::strcmp(ArgV[i], "-help") == 0 || std::strcmp(ArgV[i], "--help") == 0)
		{
			Args.bHelp = true;
		}
		else if (std::strcmp(ArgV[i], "-input") == 0 && i + 1 < ArgC)
		{
			Args.InputPath = ArgV[++i];
		}
		else if (std::strcmp(ArgV[i], "-output") == 0 && i + 1 < ArgC)
		{
			Args.OutputPath = ArgV[++i];
		}
		else if (std::strcmp(ArgV[i], "-r") == 0 && i + 1 < ArgC)
		{
			Args.MinRadiusFrac = std::atof(ArgV[++i]);
			Args.bHasMinRadius = true;
		}
		else if (std::strcmp(ArgV[i], "-t") == 0 && i + 1 < ArgC)
		{
			Args.ToleranceFrac = std::atof(ArgV[++i]);
			Args.bHasTolerance = true;
		}
		else if (std::strcmp(ArgV[i], "-stats") == 0)
		{
			Args.bStats = true;
		}
		else if (std::strcmp(ArgV[i], "-protect_unreachable") == 0)
		{
			Args.bProtectUnreachable = true;
		}
	}
	return Args;
}

static void PrintHelp()
{
	std::printf(
		"Navigation-Driven Approximate Convex Decomposition\n"
		"\n"
		"Usage: navacd -input <path.obj> -output <path.obj> -r <radius> -t <tolerance> [options]\n"
		"\n"
		"Required arguments:\n"
		"  -input <path>      Path to input OBJ mesh file\n"
		"  -output <path>     Path to output OBJ file (convex hulls with separate groups and colors)\n"
		"  -r <double>        Navigable space min radius, as a fraction of the longest bounding box axis\n"
		"  -t <double>        Navigable space tolerance, as a fraction of the longest bounding box axis\n"
		"\n"
		"Optional flags:\n"
		"  -stats             Print timing and hull count statistics\n"
		"  -protect_unreachable  Protect unreachable internal space where the min radius sphere could fit\n"
		"  -help              Print this help text\n"
	);
}

// Write the decomposition to a single OBJ file, with a separate object name and vertex coloring per convex hull
static void WriteDecompOBJ(FConvexDecomposition3& Decomp, const std::string& Path)
{
	std::ofstream FileStream(Path);
	if (!FileStream)
	{
		std::fprintf(stderr, "[Error] Failed to open output path: %s\n", Path.c_str());
		std::exit(1);
	}
	FileStream.precision(std::numeric_limits<double>::digits10);
	int32 LastV = 0;
	int32 ColorIdx = 0;

	auto DumpPart = [&FileStream, &LastV, &ColorIdx](const TArray<FIndex3i>& Tris, const TArray<FVector3d>& Verts)
	{
		const FVector3d Color = LinearColors::SelectColor<FVector3d>(ColorIdx++);
		FileStream << "o part" << ColorIdx << "\n";
		for (const FVector3d& V : Verts)
		{
			FileStream << "v " << V.X << " " << V.Y << " " << V.Z << " " << Color.X << " " << Color.Y << " " << Color.Z << "\n";
		}
		for (const FIndex3i& T : Tris)
		{
			FileStream << "f " << T.A + LastV + 1 << " " << T.C + LastV + 1 << " " << T.B + LastV + 1 << "\n";
		}
		LastV += Verts.Num();
		FileStream << "\n\n\n";
	};

	for (int32 CvxIdx = 0; CvxIdx < Decomp.NumHulls(); CvxIdx++)
	{
		TArray<FIndex3i> Tris = Decomp.GetTriangles(CvxIdx);
		TArray<FVector3d> Verts = Decomp.GetVertices<double>(CvxIdx);
		DumpPart(Tris, Verts);
	}
	FileStream.close();
}

void ExactPredicatesInit()
{
	UE::Geometry::ExactPredicates::GlobalInit();
}

int main(int ArgC, char* ArgV[])
{
	FArgs Args = ParseArgs(ArgC, ArgV);

	if (Args.bHelp)
	{
		PrintHelp();
		return 0;
	}

	// Validate required arguments
	if (Args.InputPath.empty())
	{
		std::fprintf(stderr, "[Error] Must specify -input parameter. Use -help for usage.\n");
		return 1;
	}
	if (Args.OutputPath.empty())
	{
		std::fprintf(stderr, "[Error] Must specify -output parameter. Use -help for usage.\n");
		return 1;
	}
	if (!Args.bHasMinRadius)
	{
		std::fprintf(stderr, "[Error] Must specify -r parameter. Use -help for usage.\n");
		return 1;
	}
	if (!Args.bHasTolerance)
	{
		std::fprintf(stderr, "[Error] Must specify -t parameter. Use -help for usage.\n");
		return 1;
	}

	// Initialize exact predicates
	ExactPredicatesInit();

	// Load input OBJ
	FDynamicMesh3 Mesh;
	UE::MeshFileUtils::FLoadOBJSettings LoadSettings;
	UE::MeshFileUtils::ELoadOBJStatus LoadStatus = UE::MeshFileUtils::LoadOBJ(Args.InputPath.c_str(), Mesh, LoadSettings);
	if (LoadStatus != UE::MeshFileUtils::ELoadOBJStatus::Success)
	{
		std::fprintf(stderr, "[Error] Failed to load input OBJ file: %s (status %d)\n", Args.InputPath.c_str(), (int)LoadStatus);
		return 1;
	}
	if (Mesh.TriangleCount() == 0)
	{
		std::fprintf(stderr, "[Error] Input mesh has no triangles.\n");
		return 1;
	}

	std::fprintf(stderr, "[Info] Loaded mesh: %d vertices, %d triangles\n", Mesh.VertexCount(), Mesh.TriangleCount());

	bool bIgnoreUnreachable = !Args.bProtectUnreachable;

	// Run NavACD
	double StartTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	FConvexDecomposition3 ConvexDecomposition = NavACD::Run(Mesh, Args.MinRadiusFrac, Args.ToleranceFrac, bIgnoreUnreachable);

	double EndTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	if (Args.bStats)
	{
		std::fprintf(stderr, "[Stats] Algorithm time (excluding file read/write): %.3f seconds\n", EndTime - StartTime);
		std::fprintf(stderr, "[Stats] Number of hulls: %d\n", ConvexDecomposition.NumHulls());
	}

	// Write output
	WriteDecompOBJ(ConvexDecomposition, Args.OutputPath);
	std::fprintf(stderr, "[Info] Wrote decomposition to %s\n", Args.OutputPath.c_str());

	return 0;
}
