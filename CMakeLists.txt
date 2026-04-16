cmake_minimum_required(VERSION 3.16)
project(navacd LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Shewchuk predicates compiled as C
enable_language(C)

# Collect source files
set(SHIM_SOURCES)

# GeometryCore Public headers (included via source)
set(GEOMCORE_PUBLIC
    src/GeometryCore/Public/BoxTypes.h
    src/GeometryCore/Public/PlaneTypes.h
    src/GeometryCore/Public/VectorTypes.h
    src/GeometryCore/Public/VectorUtil.h
    src/GeometryCore/Public/IndexTypes.h
    src/GeometryCore/Public/TransformTypes.h
    src/GeometryCore/Public/FrameTypes.h
    src/GeometryCore/Public/Quaternion.h
    src/GeometryCore/Public/GeometryTypes.h
    src/GeometryCore/Public/GeometryBase.h
    src/GeometryCore/Public/HalfspaceTypes.h
    src/GeometryCore/Public/LineTypes.h
    src/GeometryCore/Public/SegmentTypes.h
    src/GeometryCore/Public/SphereTypes.h
    src/GeometryCore/Public/TriangleTypes.h
    src/GeometryCore/Public/OrientedBoxTypes.h
    src/GeometryCore/Public/CircleTypes.h
    src/GeometryCore/Public/CapsuleTypes.h
    src/GeometryCore/Public/IntBoxTypes.h
    src/GeometryCore/Public/IntVectorTypes.h
    src/GeometryCore/Public/MatrixTypes.h
    src/GeometryCore/Public/MeshAdapter.h
    src/GeometryCore/Public/WeightMapTypes.h
    src/GeometryCore/Public/MeshQueries.h
    src/GeometryCore/Public/VertexConnectedComponents.h
    src/GeometryCore/Public/DynamicMesh/DynamicMesh3.h
    src/GeometryCore/Public/DynamicMesh/DynamicMeshAttributeSet.h
    src/GeometryCore/Public/DynamicMesh/DynamicMeshOverlay.h
    src/GeometryCore/Public/DynamicMesh/DynamicMeshAABBTree3.h
    src/GeometryCore/Public/DynamicMesh/MeshNormals.h
    src/GeometryCore/Public/DynamicMesh/MeshTransforms.h
    src/GeometryCore/Public/DynamicMesh/InfoTypes.h
    src/GeometryCore/Public/DynamicMesh/Operations/MergeCoincidentMeshEdges.h
    src/GeometryCore/Public/Spatial/MeshAABBTree3.h
    src/GeometryCore/Public/Spatial/FastWinding.h
    src/GeometryCore/Public/Spatial/SparseDynamicOctree3.h
    src/GeometryCore/Public/Spatial/PointHashGrid3.h
    src/GeometryCore/Public/Spatial/PriorityOrderPoints.h
    src/GeometryCore/Public/Spatial/DenseGrid3.h
    src/GeometryCore/Public/Spatial/BlockedDenseGrid3.h
    src/GeometryCore/Public/CompGeom/ConvexDecomposition3.h
    src/GeometryCore/Public/CompGeom/ConvexHull3.h
    src/GeometryCore/Public/CompGeom/Delaunay2.h
    src/GeometryCore/Public/CompGeom/ExactPredicates.h
    src/GeometryCore/Public/CompGeom/PolygonTriangulation.h
    src/GeometryCore/Private/CompGeom/ThirdParty/ShewchukPredicatesInterface.h
    src/GeometryCore/Public/Distance/DistPoint3Triangle3.h
    src/GeometryCore/Public/Generators/MarchingCubes.h
    src/GeometryCore/Public/Generators/MeshShapeGenerator.h
    src/GeometryCore/Public/Util/CompactMaps.h
    src/GeometryCore/Public/Util/DynamicVector.h
    src/GeometryCore/Public/Util/IndexUtil.h
    src/GeometryCore/Public/Util/IteratorUtil.h
    src/GeometryCore/Public/Util/RefCountVector.h
    src/GeometryCore/Public/Util/SmallListSet.h
    src/GeometryCore/Public/Util/ColorConstants.h
    src/GeometryCore/Public/Util/SizedDisjointSet.h
    src/GeometryCore/Public/Util/ProgressCancel.h
    src/GeometryCore/Public/Util/MeshCaches.h
)

# GeometryCore implementation files
set(GEOMCORE_SOURCES
    src/GeometryCore/Private/CompGeom/ConvexDecomposition3.cpp
    src/GeometryCore/Private/CompGeom/ConvexHull2.cpp
    src/GeometryCore/Private/CompGeom/ConvexHull3.cpp
    src/GeometryCore/Private/CompGeom/Delaunay2.cpp
    src/GeometryCore/Private/CompGeom/ExactPredicates.cpp
    src/GeometryCore/Private/CompGeom/PolygonTriangulation.cpp
    src/GeometryCore/Private/DynamicMesh/DynamicMesh3.cpp
    src/GeometryCore/Private/DynamicMesh/DynamicMesh3_Edits.cpp
    src/GeometryCore/Private/DynamicMesh/DynamicMesh3_Queries.cpp
    src/GeometryCore/Private/DynamicMesh/DynamicMeshAttributeSet.cpp
    src/GeometryCore/Private/DynamicMesh/DynamicMeshOverlay.cpp
    src/GeometryCore/Private/DynamicMesh/MeshIndexMappings.cpp
    src/GeometryCore/Private/DynamicMesh/MeshNormals.cpp
    src/GeometryCore/Private/DynamicMesh/MeshTransforms.cpp
    src/GeometryCore/Private/DynamicMesh/NonManifoldMappingSupport.cpp
    src/GeometryCore/Private/DynamicMesh/Operations/MergeCoincidentMeshEdges.cpp
    src/GeometryCore/Private/Generators/MarchingCubes.cpp
    src/GeometryCore/Private/Intersection/IntersectionQueries3.cpp
    # src/GeometryCore/Private/Spatial/SparseDynamicOctree3.cpp  # stripped: header is now a linear-scan stub
    src/GeometryCore/Private/Spatial/PriorityOrderPoints.cpp
    src/GeometryCore/Private/ThirdParty/Shewchuk/Predicates.cpp
    src/GeometryCore/Private/Util/ColorConstants.cpp
    src/GeometryCore/Private/Util/IndexUtil.cpp
    src/GeometryCore/Private/Util/ProgressCancel.cpp
    src/GeometryCore/Private/Util/SmallListSet.cpp
    src/GeometryCore/Private/Util/UniqueIndexSet.cpp
)

# IO sources
set(IO_SOURCES
    src/IO/OBJMeshUtil.cpp
)

# Application sources
set(APP_SOURCES
    src/main.cpp
    src/NavACD.cpp
)

add_executable(navacd
    ${APP_SOURCES}
    ${IO_SOURCES}
    ${GEOMCORE_SOURCES}
)

target_include_directories(navacd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Shim
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GeometryCore/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GeometryCore/Private
)

# Shewchuk predicates need specific defines
set_source_files_properties(
    src/GeometryCore/Private/ThirdParty/Shewchuk/Predicates.cpp
    PROPERTIES
    LANGUAGE C
    COMPILE_DEFINITIONS "REDUCED;NO_TIMER"
)

# Disable some warnings for extracted UE code
if(MSVC)
    target_compile_options(navacd PRIVATE /W3 /wd4244 /wd4456 /wd4458 /wd4459 /wd4018 /wd4267 /wd4800 /wd4996 /wd4702 /wd4723 /wd4624 /wd4275)
    # Force UTF-8 source encoding
    target_compile_options(navacd PRIVATE /utf-8)
    # Force-include UECompat.h so all UE types are available
    target_compile_options(navacd PRIVATE /FI"${CMAKE_CURRENT_SOURCE_DIR}/src/Shim/UECompat.h")
else()
    target_compile_options(navacd PRIVATE -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare -Wno-class-memaccess -Wno-overloaded-virtual)
    target_compile_options(navacd PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/src/Shim/UECompat.h>)
endif()

# Threading support (for future OpenMP)
find_package(Threads REQUIRED)
target_link_libraries(navacd PRIVATE Threads::Threads)
