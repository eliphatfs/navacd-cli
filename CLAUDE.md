# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A standalone Linux CLI (`navacd`) that extracts Unreal Engine's NavACD (Navigation Approximate Convex Decomposition) algorithm out of UE and runs it on a single OBJ mesh. The source was lifted from UE5's `Runtime/GeometryCore` and made to compile without the rest of UE via a hand-written shim.

## Build & run

```sh
cmake -S /project -B /project/build -G "Unix Makefiles"
cmake --build /project/build -j$(nproc)
```

Binary: `/project/build/navacd`. Linux-only, gcc, C++20. No test suite — validate end-to-end by running on a concave OBJ and checking exit 0 + non-empty OBJ output.

```sh
navacd -input mesh.obj -output hulls.obj -r 0.1 -t 0.01 [-stats] [-protect_unreachable]
```

All four of `-input`, `-output`, `-r`, `-t` are required. `-r` (min navigable radius) and `-t` (tolerance) are fractions of the longest bbox axis. `-stats` prints timing + hull count. `-protect_unreachable` preserves interior voids where a min-radius sphere fits.

## Architecture (three layers, bottom-up)

1. **`src/Shim/`** — hand-written UE compat layer replacing `Core` / `CoreUObject` for the subset GeometryCore needs. Master header is `src/Shim/UECompat.h`; subdirs include `Containers/`, `Math/`, `Templates/`, `Misc/`, `HAL/`, `Serialization/`, `Logging/`, `Async/`, `Tasks/`, `UObject/`, `Implicit/`.
2. **`src/GeometryCore/`** — pruned extraction of UE5's `Runtime/GeometryCore` (`Public/` headers, `Private/` impls). Hot path: `DynamicMesh3`, `ConvexHull3`, `ConvexDecomposition3`, `FastWinding`, `MarchingCubes`, `MeshAABBTree3`, `SmallListSet`. Shewchuk predicates under `Private/ThirdParty/Shewchuk/` compile as C.
3. **`src/` (top)** — `main.cpp`, `NavACD.{h,cpp}` (algorithm driver), `IO/` (OBJ reader/writer).

### Build mechanic worth knowing

`CMakeLists.txt` force-includes `src/Shim/UECompat.h` into every C++ TU using a **CXX-only generator expression**:

```
$<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/src/Shim/UECompat.h>
```

The CXX guard is load-bearing: `Predicates.cpp` compiles as C and must not see the C++ shim. Don't collapse it into a plain `-include`.

## Aggressively stripped UE subsystems

NavACD doesn't use these, so they were deleted rather than shimmed. Re-introducing any of them is out-of-scope and breaks the build:

- Mesh serialization (`DynamicMesh3_Serialization.cpp` exists on disk but is excluded from the build).
- Bone attributes, skin weights, morph targets, sculpt layers.
- Polygroup / weight / material / triangle-label attribute templates.
- `SparseDynamicOctree3` — replaced by a linear-scan stub header.

If a compile error references one of these, **prefer deleting the caller** over reviving the subsystem.

## Shim invariants that are easy to break

- **`TUniquePtr` does NOT inherit from `std::unique_ptr`** (`src/Shim/Templates/UniquePtr.h`). It holds a raw pointer plus a function-pointer deleter bound only at `Reset()` / construction. This lets `TUniquePtr<IncompleteT>` appear as a default-initialized class member — UE patterns rely on this, and `std::unique_ptr`'s destructor would static_assert a complete type. Don't "simplify" it back.
- **`HAL/` is uppercase.** Linux FS is case-sensitive; UE headers include `HAL/Platform.h`. Don't rename to `Hal/`.
- **`FMemory` uses `posix_memalign` / `std::malloc`** — Linux-only. No `_aligned_malloc`.
- **NavACD's negative-space sampler uses the real `FMarchingCubes`.** `ConvexDecomposition3.cpp` constructs `FMarchingCubes` to mesh the empty region inside the convex hull but outside the source mesh; sphere samples in that region drive the split decisions. If that meshing silently no-ops, every input collapses to a single hull. The real class lives at `GeometryCore/Public/Generators/MarchingCubes.h` and its `.cpp` lookup tables must be in `GEOMCORE_SOURCES`. Do not re-add a shim stub for it.
- **`ExactPredicatesInit()` in `main.cpp` MUST call `UE::Geometry::ExactPredicates::GlobalInit()`.** The Shewchuk library is linked (`Predicates.cpp` is in `GEOMCORE_SOURCES`), but its adaptive predicates read static error-bound constants (`epsilon`, `splitter`, `o3derrboundA/B/C`, etc.) that are zero until `exactinit()` runs. Skipping init silently routes every `Orient3D` through the inexact path; simple meshes tolerate it, dense/near-coplanar inputs corrupt `ConvexHull3` connectivity and assert in `UpdateNeighbor`. Do not re-stub this function.
- **`FCriticalSection` is movable (composition, not inheritance).** It holds a `unique_ptr<std::mutex>` so that `TArray<FCriticalSection>` / `std::vector<FCriticalSection>` can instantiate `resize`/`reserve` (`std::mutex` itself is neither copyable nor movable). MarchingCubes relies on this.

## Where to add code

- Missing UE type referenced by GeometryCore → add a minimum stub to `src/Shim/UECompat.h` or a targeted subheader. **Never add new UE source files.**
- Missing GeometryCore translation unit surfacing at link time → add it to `GEOMCORE_SOURCES` in `CMakeLists.txt`.
- Tie-breaker: if a symbol needs more than ~20 lines of new shim to satisfy, strip the caller instead.
