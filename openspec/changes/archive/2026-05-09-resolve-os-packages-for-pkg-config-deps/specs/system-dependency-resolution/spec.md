# System Dependency Resolution

## Purpose

Resolve transitive dependencies for system libraries by using a linked `pkg-config` library API. The resolution flow SHALL pass through OS package enrichment and deduplication before components are finalized.

## MODIFIED Requirements

### Requirement: System Dependency Resolution — Enriched Resolution Flow
The system SHALL use a linked `pkg-config` library API to retrieve transitive dependencies for components that are identified as `pkgconfig` type in the Meson introspection data, or for external components where Meson subproject introspection files are unavailable. Resolved dependencies SHALL be passed through OS package resolution and deduplication before SBOM component creation.

#### Scenario: Successful system dependency resolution via type
- **WHEN** a dependency is listed in the Meson introspection data with `"type": "pkgconfig"`
- **THEN** the system uses the `pkg-config` library to retrieve the requirements of that package
- **AND** the discovered transitive dependencies are collected into the raw dependency list
- **AND** after collection, OS package resolution is attempted for each dependency
- **AND** deduplication by OS package is applied
- **AND** the final components are created from the resolved, deduplicated list

#### Scenario: Successful system dependency resolution via fallback
- **WHEN** a dependency is not found in the build directory's subproject introspection data
- **THEN** the system attempts to use the `pkg-config` library to retrieve the requirements of that package
- **AND** if successful, the discovered transitive dependencies are collected into the raw dependency list
- **AND** after collection, OS package resolution is attempted for each dependency
- **AND** deduplication by OS package is applied

#### Scenario: Dependencies from pkg-config include .pc file path
- **WHEN** a dependency is resolved via pkg-config
- **THEN** the system SHALL record the `.pc` file path from `pkgconf_pkg_t.filename` alongside the dependency name and version
- **AND** the `.pc` file path SHALL be used later for OS package resolution

#### Scenario: Handling non pkg-config dependencies unchanged
- **WHEN** a dependency is neither found in Meson subproject introspection data nor available via the `pkg-config` library
- **THEN** the system SHALL issue a warning to `stderr` indicating that the dependency could not be resolved
- **AND** the system MUST continue the resolution process for other dependencies

#### Scenario: Resolution via Meson bridge before pkg-config fallback
- **WHEN** a dependency is of type `"pkgconfig"`
- **AND** the dependency has no subproject introspection files
- **AND** its entry in the main `intro-dependencies.json` contains a `"dependencies"` array with child names
- **THEN** the system SHALL use those children from the `dependencies` array as the resolved transitive deps
- **AND** the pkg-config fallback SHALL NOT be attempted for this dependency
- **AND** self-references (a component listing itself as its own dependency) SHALL be silently skipped

#### Scenario: Complete resolution order
- **WHEN** a transitive dependency is being resolved
- **THEN** the system SHALL check in this order:
    1. Subproject introspection (`subprojects/<name>/intro-dependencies.json`)
    2. Meson bridge (`intro-dependencies.json` entry's `dependencies` field)
    3. pkg-config `Requires` via linked library
    4. Warning if none found