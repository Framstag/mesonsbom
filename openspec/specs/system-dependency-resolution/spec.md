# system-dependency-resolution Specification

## Purpose
Resolve transitive dependencies for system libraries by using a linked `pkg-config` library API, avoiding shell command invocations for portability and security.

## Requirements

### Requirement: System Dependency Resolution
The system SHALL use a linked `pkg-config` library API to retrieve transitive dependencies for components that are identified as `pkgconfig` type in the Meson introspection data, or for external components where Meson subproject introspection files are unavailable.

#### Scenario: Successful system dependency resolution via type
- **WHEN** a dependency is listed in the Meson introspection data with `"type": "pkgconfig"`.
- **THEN** the system uses the `pkg-config` library to retrieve the requirements of that package.
- **AND** the discovered transitive dependencies are added to the SBOM components and dependency edges.

#### Scenario: Successful system dependency resolution via fallback
- **WHEN** a dependency is not found in the build directory's subproject introspection data.
- **THEN** the system attempts to use the `pkg-config` library to retrieve the requirements of that package.
- **AND** if successful, the discovered transitive dependencies are added to the SBOM components and dependency edges.

#### Scenario: Handling non pkg-config dependencies
- **WHEN** a dependency is neither found in Meson subproject introspection data nor available via the `pkg-config` library.
- **THEN** the system SHALL issue a warning to `stderr` indicating that the dependency could not be resolved.
- **AND** the system MUST continue the resolution process for other dependencies.
