## 1. Setup

- [x] 1.1 Initialize a new Meson project for the `mesonsbom` binary

- [x] 1.2 Add required dependencies to `meson.build`:
  - `nlohmann_json` (header‑only JSON parser)
  - `cxxopts` (command‑line option parser)

## 2. Core Implementation

- [x] 2.1 Implement CLI parsing to accept `--build-dir <path>` using `cxxopts`
- [x] 2.2 Load and validate the required Meson introspection JSON files (`projectinfo.json`, `dependencies.json`, etc.) from the expected `meson-info` subdirectory (no recursive search).
- [x] 2.3 Implement SBOMBuilder class (based on nlohmann::json) and use it to construct the CycloneDX SBOM
- [x] 2.4 Generate CycloneDX 1.6 SBOM (JSON) using SBOMBuilder
- [x] 2.5 Write the SBOM to `sbom.json` in the current working directory, handling errors gracefully

## 3. Build & CI Integration

- [x] 3.1 Add a Meson build target for the `mesonsbom` binary

- [x] 3.2 Create a CI pipeline step that runs the generator after a successful build and archives the resulting `sbom.json` artifact

## 4. Testing

- [x] 4.1 Write unit tests for JSON loading and component mapping logic
- [x] 4.2 Write an integration test using a sample Meson build directory to verify end‑to‑end SBOM generation
