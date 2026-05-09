# Meson Build System Tool Metadata in `metadata.tools.components`

## Purpose

The system SHALL enrich the SBOM's `metadata.tools.components` with a CycloneDX Component object representing the Meson build system that built the analyzed project. This enables consumers to identify which build tool and version produced the software artifacts.

## Requirements

### Requirement: Meson tool metadata from build directory
The generated SBOM SHALL include a meson tool component in `metadata.tools.components` when `meson-info/meson-info.json` is present and readable in the analyzed project's build directory. The version SHALL be taken from `meson_version.full` in that file.

#### Scenario: Meson tool present when meson-info.json exists
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **AND** `/path/to/build/meson-info/meson-info.json` exists with `"meson_version": {"full": "1.11.1"}`
- **THEN** `metadata.tools.components` SHALL contain an entry with `name` equal to `"meson"`
- **AND** that entry SHALL have `version` equal to `"1.11.1"`

### Requirement: Meson appears as second entry in tools.components
The meson tool entry SHALL appear as the second entry in `metadata.tools.components`, immediately after `mesonsbom`.

#### Scenario: Meson is second tool in the array
- **WHEN** an SBOM is generated with a valid build directory containing `meson-info.json`
- **THEN** the second entry in `metadata.tools.components` SHALL have `name` equal to `"meson"`

### Requirement: Meson tool component has bom-ref and purl
The meson tool component SHALL include a `bom-ref` and a `purl` field.

#### Scenario: Meson tool component has bom-ref and purl
- **WHEN** an SBOM is generated that includes the meson tool
- **THEN** the meson tool component SHALL have a `bom-ref` field with value `"tool-meson"`
- **AND** the meson tool component SHALL have a `purl` field with value `"pkg:generic/meson@<version>"` where `<version>` matches `meson_version.full`

### Requirement: Meson tool component includes supplier information
The meson tool component SHALL include a `supplier` object identifying the Meson development team.

#### Scenario: Meson tool component has supplier
- **WHEN** an SBOM includes the meson tool
- **THEN** the meson tool component SHALL contain a `supplier` object with `name` set to `"The Meson Development Team"`

### Requirement: Meson tool component includes license information
The meson tool component SHALL include a `licenses` array with the Meson project's SPDX license identifier.

#### Scenario: Meson tool component has licenses
- **WHEN** an SBOM includes the meson tool
- **THEN** the meson tool component SHALL contain a `licenses` array with `{"license": {"id": "Apache-2.0"}}`

### Requirement: Meson tool component includes external references
The meson tool component SHALL include an `externalReferences` array with the Meson project homepage and VCS URLs.

#### Scenario: Meson tool component has external references
- **WHEN** an SBOM includes the meson tool
- **THEN** the meson tool component SHALL contain an `externalReferences` array
- **AND** one entry SHALL have `type` equal to `"website"` and `url` equal to `"https://mesonbuild.com"`
- **AND** one entry SHALL have `type` equal to `"vcs"` and `url` equal to `"git+https://github.com/mesonbuild/meson.git"`

### Requirement: Meson tool component includes description
The meson tool component SHALL include a `description` field summarizing the tool's purpose.

#### Scenario: Meson tool component has description
- **WHEN** an SBOM includes the meson tool
- **THEN** the meson tool component SHALL have a `description` field set to `"Meson is a project to create the best possible next-generation build system"`

### Requirement: Meson tool component type
The meson tool component SHALL have `type` set to `"application"`.

#### Scenario: Meson tool component type is application
- **WHEN** an SBOM includes the meson tool
- **THEN** the meson tool entry SHALL have `type` equal to `"application"`

### Requirement: Graceful degradation on missing meson-info.json
If `meson-info.json` cannot be opened or parsed, the system SHALL emit a warning to stderr and continue generating the SBOM without the meson tool entry.

#### Scenario: Missing meson-info.json produces warning but continues
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **AND** `/path/to/build/meson-info/meson-info.json` does not exist or is unreadable
- **THEN** the tool SHALL print a warning to stderr
- **AND** the tool SHALL generate a valid SBOM without a meson tool entry
- **AND** the tool SHALL exit with code 0

#### Scenario: Corrupt meson-info.json produces warning but continues
- **WHEN** `meson-info.json` contains invalid JSON
- **THEN** the tool SHALL print a warning to stderr
- **AND** the tool SHALL generate a valid SBOM without a meson tool entry
- **AND** the tool SHALL exit with code 0
