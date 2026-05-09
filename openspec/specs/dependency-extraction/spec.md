# Dependency Extraction

## Purpose

The system shall extract direct library dependencies from the Meson build introspection files and include them in the generated CycloneDX SBOM.
## Requirements
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

#### Scenario: Two-phase collection for OS resolution
- **WHEN** dependencies are extracted from `intro-dependencies.json`
- **THEN** each dependency SHALL first be collected into a raw dependency list
- **AND** for dependencies of type `pkgconfig`, the system SHALL also record the `.pc` file path from `pkgconf_pkg_t.filename`
- **AND** component creation in the SBOM SHALL NOT occur until after OS resolution and deduplication is complete

#### Scenario: Meson bridge deps in BFS collection
- **WHEN** a dependency entry in `intro-dependencies.json` has a `"dependencies"` array listing child dependencies (Meson bridge)
- **AND** the child dependency is not found via subproject introspection
- **AND** its pkg-config `.pc` file has no `Requires` that includes it
- **THEN** the system SHALL still discover the child dependency from the `dependencies` array
- **AND** it SHALL be added to the raw dependency list with `type: "pkgconfig"` for OS resolution

