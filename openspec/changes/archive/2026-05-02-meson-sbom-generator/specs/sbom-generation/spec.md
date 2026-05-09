## ADDED Requirements

### Requirement: SBOM Generation from Meson Build Metadata
The system SHALL generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory.

#### Scenario: Successful SBOM generation
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **THEN** the tool reads `projectinfo.json` and `dependencies.json` (and any other required Meson introspection files) and writes a valid CycloneDX 1.6 SBOM to `sbom.json` in the current working directory.
