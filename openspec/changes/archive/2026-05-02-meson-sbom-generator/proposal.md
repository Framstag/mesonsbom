## Why

Generating a CycloneDX SBOM directly from a Meson build directory provides an automated way to capture the complete dependency graph of a compiled application, improving supply‑chain visibility and compliance without requiring manual inventory.

## What Changes

- Add a new C++ command‑line application `mesonsbom`.

- The tool reads pre‑generated Meson introspection JSON files (`projectinfo.json`, `dependencies.json`, etc.) from the expected `meson-info` subdirectory of the provided build directory (no recursive search).
- It produces a CycloneDX 1.6 SBOM in JSON format.
- Uses a header‑only JSON library (`nlohmann::json`) and an internal `SBOMBuilder` class to construct the CycloneDX 1.6 SBOM.
- Build system for the tool is Meson.

## Capabilities

### New Capabilities
- `sbom-generation`: Generates a CycloneDX SBOM from Meson build metadata.

### Modified Capabilities
- *(none)*

## Impact

- Introduces a new binary dependency (the SBOM generator) into the repository.
- Adds Meson as a build requirement for the tool itself.
- Provides CI pipelines with an artifact (`sbom.json`) for compliance checks.
