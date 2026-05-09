# Dependency Extraction

## Purpose

The system shall extract direct library dependencies from the Meson build introspection files. Dependencies SHALL be collected in a raw phase first, before OS resolution and deduplication.

## MODIFIED Requirements

### Requirement: Dependency Extraction — Two-Phase Collection
The system SHALL read `intro-dependencies.json` produced by `meson introspect` and collect each listed direct library into a raw dependency list. The raw list SHALL include the dependency name, version, pkg-config `.pc` file path (where available), and description. OS resolution and component creation SHALL occur in a separate phase after all dependencies are collected.

#### Scenario: Dependencies collected in raw phase
- **WHEN** the `mesonsbom` binary is executed with `--build-dir <path>` pointing at a Meson build directory containing `meson-info/intro-dependencies.json`
- **THEN** each entry in `intro-dependencies.json` SHALL be collected into a raw dependency list
- **AND** for dependencies of type `pkgconfig`, the system SHALL also record the `.pc` file path from `pkgconf_pkg_t.filename`
- **AND** the system SHALL trigger transitive dependency resolution for each collected dependency
- **AND** component creation in the SBOM SHALL NOT occur until after OS resolution and deduplication is complete

#### Scenario: Component created after OS resolution
- **WHEN** the raw dependency list has been fully collected
- **AND** OS resolution has completed for each entry
- **AND** deduplication by OS package has been applied
- **THEN** the system SHALL create SBOM components from the resolved, deduplicated list
- **AND** dependency edges SHALL reference the final component identities (which may differ from raw pkg-config names)

#### Scenario: Meson bridge deps in BFS collection
- **WHEN** a dependency entry in `intro-dependencies.json` has a `"dependencies"` array listing child dependencies (Meson bridge)
- **AND** the child dependency is not found via subproject introspection
- **AND** its pkg-config `.pc` file has no `Requires` that includes it
- **THEN** the system SHALL still discover the child dependency from the `dependencies` array
- **AND** it SHALL be added to the raw dependency list with `type: "pkgconfig"` for OS resolution