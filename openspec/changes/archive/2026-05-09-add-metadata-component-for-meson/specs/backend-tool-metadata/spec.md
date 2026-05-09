# Backend Build Tool Metadata in `metadata.tools.components`

## Purpose

The system SHALL enrich the SBOM's `metadata.tools.components` with a CycloneDX Component object representing the Meson backend build tool (e.g., ninja, Visual Studio, Xcode) that was used to compile the analyzed project. This enables consumers to identify which build backend was involved in producing the software artifacts.

## Requirements

### Requirement: Backend tool detected from intro-buildoptions.json
The generated SBOM SHALL include a backend tool component in `metadata.tools.components` when `intro-buildoptions.json` is present and readable, and the `backend` option is set to a known value (`ninja`, `vs*`, `xcode`).

#### Scenario: Backend tool present when intro-buildoptions.json exists
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **AND** `/path/to/build/meson-info/intro-buildoptions.json` exists with an entry `{"name": "backend", "value": "ninja"}`
- **THEN** `metadata.tools.components` SHALL contain an entry with `name` equal to `"ninja"`

### Requirement: Backend appears as third entry in tools.components
The backend tool entry SHALL appear as the third entry in `metadata.tools.components`, immediately after Meson.

#### Scenario: Backend is third tool in the array
- **WHEN** an SBOM is generated with a valid build directory containing all meson-info files
- **THEN** the third entry in `metadata.tools.components` SHALL contain a `name` matching the backend name from `intro-buildoptions.json`

### Requirement: Backend version defaults to unknown
The backend tool component SHALL have version set to `"unknown"` since the build directory does not contain the backend's actual installed version.

#### Scenario: Backend tool has unknown version
- **WHEN** an SBOM includes a backend tool entry
- **THEN** the backend entry SHALL have a `version` field with value `"unknown"`

### Requirement: Backend tool has bom-ref and purl
The backend tool component SHALL include a `bom-ref` and a `purl` field.

#### Scenario: Backend tool has bom-ref and purl
- **WHEN** an SBOM includes a backend tool entry with name `"ninja"`
- **THEN** the backend entry SHALL have a `bom-ref` field with value `"tool-ninja"`
- **AND** the backend entry SHALL have a `purl` field with value `"pkg:generic/ninja@unknown"`

### Requirement: Known backend has metadata
For known backends (ninja, vs*, xcode), the backend tool component SHALL include description, supplier, homepage, VCS, and license information where available.

#### Scenario: Ninja backend has complete metadata
- **WHEN** an SBOM includes a ninja backend entry
- **THEN** the entry SHALL contain a `description` field
- **AND** the entry SHALL contain a `supplier` object with `name` set to `"The Ninja Project"`
- **AND** the entry SHALL contain a `licenses` array with `{"license": {"id": "Apache-2.0"}}`
- **AND** the entry SHALL contain `externalReferences` with type `"website"` url `"https://ninja-build.org"` and type `"vcs"` url `"git+https://github.com/ninja-build/ninja.git"`

### Requirement: Graceful degradation on missing intro-buildoptions.json
If `intro-buildoptions.json` cannot be opened or the `backend` option is missing/unknown, the system SHALL silently skip adding the backend tool entry without a warning.

#### Scenario: Missing intro-buildoptions.json skips backend silently
- **WHEN** `intro-buildoptions.json` does not exist
- **THEN** the tool SHALL generate a valid SBOM without a backend tool entry
- **AND** the tool SHALL NOT print a warning about the missing file
- **AND** the tool SHALL exit with code 0

#### Scenario: Unknown backend value skips backend silently
- **WHEN** `intro-buildoptions.json` contains `{"name": "backend", "value": "none"}`
- **THEN** the tool SHALL generate a valid SBOM without a backend tool entry
