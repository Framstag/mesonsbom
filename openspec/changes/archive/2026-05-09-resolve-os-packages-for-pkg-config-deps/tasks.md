## 1. Foundation — New Classes and File Accessors

- [x] 1.1 [2] Add `getPackageFilename()` method to `PkgConfigWrapper` (exposes `pkgconf_pkg_t.filename`), with unit test verifying path is returned (ref: os-package-resolution/spec.md, design.md Decision 2)
- [x] 1.2 [2] Create `src/os_release.h` and `src/os_release.cpp` with `OsRelease` struct (id, name, versionId) and a `parseOsRelease()` function reading `/etc/os-release`, with unit test for successful parse and missing-file fallback (ref: os-release-detection/spec.md)
- [x] 1.3 [1] Add `OsRelease` files to `meson.build` executable and test targets (ref: os-release-detection/spec.md)

## 2. Core — OS Package Resolver

- [x] 1.4 [3] Create `src/os_package_resolver.h` and `src/os_package_resolver.cpp` with `OsPackageResolver` class: `detectPackageManager()` checks for `pacman`, `dpkg-query`, `rpm`, `apk` in `$PATH` in order, with unit test for each PM detection scenario (ref: os-release-detection/spec.md, design.md Decision 2)
- [x] 1.5 [5] Implement `OsPackageResolver::resolve(pcFilePath)` — spawn subprocess via `popen()` with PM-specific command (`pacman -Qo`, `dpkg -S`, `rpm -qf`, `apk info --who-owns`), parse output to extract OS package name and version, cache result by file path, with unit test using mocked subprocess output (ref: os-package-resolution/spec.md, design.md Decision 2)
- [x] 1.6 [2] Add `OsPackageResolver` files to `meson.build` executable and test targets (ref: os-package-resolution/spec.md)

## 3. SBOMBuilder — Evidence, Properties, and purl Qualifier

- [x] 2.1 [3] Add `addEvidenceIdentity()` and `addEvidenceOccurrence()` methods to `SBOMBuilder` — builds `evidence.identity[]` array and `evidence.occurrences[]` array per CycloneDX 1.6 schema, with unit test verifying JSON output structure (ref: sbom-generation/spec.md, design.md Decision 4)
- [x] 2.2 [2] Add `addProperties()` method to `SBOMBuilder` — appends name-value pairs to `properties[]` array, with unit test verifying output (ref: sbom-generation/spec.md, design.md Decision 4)
- [x] 2.3 [2] Modify purl generation in `SBOMBuilder` to accept optional OS qualifier — change `makePurl()` or add overload that appends `?os=<pm>` when OS package data is present, with unit test verifying purl format (ref: sbom-generation/spec.md, design.md Decision 3)
- [x] 2.4 [1] Add `setSupplier()` method to `SBOMBuilder::addComponent()` or via a separate `setSupplier()` for the component level, with unit test (ref: sbom-generation/spec.md)

## 4. Pipeline Restructuring — Two-Phase main.cpp Rewrite

- [x] 3.1 [5] Restructure `main.cpp`: replace inline BFS with Phase 1 (Collect) that walks the entire dep graph and stores results in a `std::vector<RawDependency>` (using data structures from design.md), preserving all existing BFS logic (circular detection, optional deps filtering, target filtering) (ref: dependency-extraction/spec.md, design.md Decision 1)
- [x] 3.2 [3] Add Phase 2 (Resolve) to `main.cpp`: iterate `RawDependency` list, call `OsPackageResolver::resolve()` for pkg-config deps, store result as `ResolvedDependency` (ref: os-package-resolution/spec.md, design.md Decision 1)
- [x] 3.3 [3] Add Phase 3 (Deduplicate) to `main.cpp`: group `ResolvedDependency` by `osName`, merge evidence for duplicates, keep non-OS-resolved items as individual entries (ref: os-package-resolution/spec.md, design.md Decision 1)
- [x] 3.4 [5] Add Phase 4 (Build) to `main.cpp`: feed resolved list to `SBOMBuilder` with OS identity, evidence, properties, and updated dependency edges (ref: dependency-extraction/spec.md, sbom-generation/spec.md, design.md Decision 1)
- [x] 3.5 [2] Parse `/etc/os-release` via `OsRelease` in `main.cpp` and pass distribution name to supplier when building OS-enriched components (ref: os-release-detection/spec.md, sbom-generation/spec.md)
- [x] 3.6 [3] Add Meson bridge fallback in `loadDependencies()`: before falling back to pkg-config's `Requires`, check `intro-dependencies.json` entry's `dependencies` field for Meson-discovered children (Qt5Svg pattern). Filter out self-references. (ref: system-dependency-resolution/spec.md, dependency-extraction/spec.md, design.md Decision 7)
- [x] 3.7 [3] Filter out deps from `intro-dependencies.json` that no target references in `intro-targets.json` (Qt6Core discovered but unused pattern) (ref: sbom-generation/spec.md)

## 5. Tests — Verification of Fallback, Deduplication, and Integration

- [x] 4.1 [2] Add unit test for OS resolution failure fallback: simulate missing package manager, verify component uses pkg-config identity with no evidence/properties (ref: os-package-resolution/spec.md)
- [x] 4.2 [3] Add unit test for deduplication: two pkg-config deps map to same OS package, verify single component with consolidated evidence and both `.pc` paths in occurrences (ref: os-package-resolution/spec.md, sbom-generation/spec.md)
- [x] 4.3 [2] Add unit test for purl format: verify `pkg:generic/name@ver` for pkg-config-only deps, `pkg:generic/name@ver?os=pm` for OS-resolved deps (ref: sbom-generation/spec.md)
- [x] 4.4 [3] Add unit test for evidence structure: verify `evidence.identity[]` has correct field/concludedValue/confidence/methods per CycloneDX schema (ref: sbom-generation/spec.md)
- [x] 4.5 [3] Add unit test for properties structure: verify all six required properties present with correct values (ref: sbom-generation/spec.md)
- [x] 4.6 [3] Update existing integration test (`integration_test.cpp`) to produce SBOM with real pkg-config deps, verify OS resolution or fallback happens, verify no JSON schema violations (ref: all specs)
- [x] 4.7 [3] Add unit test for Meson bridge deps (`test_dep_bridge.cpp`): verify `loadDependencies` returns children from `intro-dependencies.json` `"dependencies"` field when pkg-config has no `Requires`, and excludes self-references (ref: system-dependency-resolution/spec.md, dependency-extraction/spec.md)

## 6. Build and Cleanup

- [x] 5.1 [1] Build project and verify all tests pass (ref: all specs)
- [x] 5.2 [1] Verify existing CLI flags (`--build-dir`, `--output`, `--target`, `--version`, `--help`) behave identically to before (ref: sbom-generation/spec.md)

## 7. Scripts

- [x] 7.1 [2] Create `diagnose_sbom.sh` — full SBOM diagnostic with OS status listing, duplicate detection, unknown version warnings
- [x] 7.2 [3] Create `test_os_resolution.sh` — integration tests for nested chain, Qt dedup, single dep, Meson bridge deps
- [x] 7.3 [1] Create `trace_os_origin.sh` — trace which pkg-config dep mapped to an OS package

## 8. Documentation Update

- [x] 8.1 [1] Update `design.md` with Meson bridge deps (Decision 7) and resolution order
- [x] 8.2 [1] Update `system-dependency-resolution/spec.md` with bridge fallback scenarios
- [x] 8.3 [1] Update `dependency-extraction/spec.md` with Meson bridge in BFS collection
