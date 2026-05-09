## MODIFIED Requirements

### Requirement: Dependency Extraction
The system SHALL read `intro-dependencies.json` produced by `meson introspect` and add each listed direct library as a CycloneDX component of type **library** with its name and version. Each component must also include a unique `bom-ref` identifier (use the component name as the reference).

#### Scenario: Successful extraction of a library
- **WHEN** the `mesonsbom` binary is executed with `--build-dir <path>` pointing at a Meson build directory containing `meson-info/intro-dependencies.json`.
- **THEN** the generated `sbom.json` includes a component for each entry in `intro-dependencies.json` with:
  - `"type": "library"`
  - `"name"` matching the entry’s `name`
  - `"version"` matching the entry’s `version`
  - `"bom-ref"` set to the component name.
- **AND** a dependency edge from the main application component to each library component is present, using the `bom-ref` values for both `ref` and `dependsOn` fields.
- **AND** the system triggers the `transitive-dependency-resolution` process to resolve further dependencies for each extracted library.
