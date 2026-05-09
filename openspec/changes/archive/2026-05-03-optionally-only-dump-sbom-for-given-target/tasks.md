## 1. CLI Changes

- [x] 1.1 Add `-t` / `--target` option to `cxxopts` in `src/main.cpp`.
- [x] 1.2 After parsing CLI, when `--target` is specified: load `intro-targets.json`, find the matching target by `name`, and use that name as the main component name.
- [x] 1.3 If the target is not found in `intro-targets.json`, print an error with available target names and exit with code 1.

## 2. Dependency Scoping

- [x] 2.1 When `--target` is specified, filter the direct dependency list (`intro-dependencies.json`) to only include entries whose `name` matches the target's `dependencies` array from `intro-targets.json`.
- [x] 2.2 Ensure transitive resolution (BFS) only proceeds from the filtered direct dependencies.

## 3. Testing & Verification

- [x] 3.1 Build the project and verify `--target` is accepted as a CLI option.
- [x] 3.2 Run `mesonsbom --build-dir build --target mesonsbom` and verify the SBOM uses `"mesonsbom"` as the main component name and only includes relevant dependencies.
- [x] 3.3 Run `mesonsbom --build-dir build --target mesonsbom_tests` and verify a different target scoping.
- [x] 3.4 Run `mesonsbom --build-dir build` (without `--target`) and verify full-project SBOM is unchanged.
- [x] 3.5 Run `mesonsbom --build-dir build --target nonexistent` and verify error message with available targets.
- [x] 3.6 Run the full test suite to ensure no regressions.
