# Tool Metadata in CycloneDX `metadata.tools.components`

## Purpose

The system SHALL emit tool metadata in the CycloneDX 1.5+ `metadata.tools.components` format (full Component objects), replacing the deprecated flat `metadata.tools` array entirely. This enables richer tool identification with supplier, license, and external reference information.

## ADDED Requirements

### Requirement: Tool metadata in tools.components format
The generated SBOM SHALL place tool identity information in `metadata.tools.components` as an array of full Component objects, per the CycloneDX 1.5+ schema. The deprecated flat `metadata.tools` array format SHALL NOT be used.

#### Scenario: Tools emitted as Component objects in tools.components
- **WHEN** an SBOM is generated
- **THEN** `metadata.tools` SHALL be an object with a `components` array
- **AND** each entry in `components` SHALL be a CycloneDX Component object with fields: `type`, `name`, `version`, `bom-ref`, `purl`

### Requirement: mesonsbom appears first in tools.components array
mesonsbom SHALL always be the first entry in the `metadata.tools.components` array, regardless of how many other tools are added in the future.

#### Scenario: mesonsbom is first tool in the array
- **WHEN** an SBOM is generated
- **THEN** the first entry in `metadata.tools.components` SHALL have `name` equal to `"mesonsbom"`
- **AND** subsequent entries (if any) SHALL appear after mesonsbom

### Requirement: Tool component includes bom-ref and purl
Each tool component in `tools.components` SHALL include a `bom-ref` and a `purl` field.

#### Scenario: Tool component has bom-ref and purl
- **WHEN** an SBOM is generated
- **THEN** each tool component SHALL have a `bom-ref` field with format `"tool-<name>"`
- **AND** each tool component SHALL have a `purl` field with format `"pkg:generic/<lowercase-name>@<version>"`

### Requirement: Tool component includes supplier information
Each tool component in `tools.components` SHALL include a `supplier` object with the author/organization name.

#### Scenario: Tool component has supplier
- **WHEN** an SBOM is generated
- **THEN** the mesonsbom tool component SHALL contain a `supplier` object with `name` set to `"Tim Teulings"`

### Requirement: Tool component includes license information
Each tool component in `tools.components` SHALL include a `licenses` array with the tool's SPDX license identifier(s).

#### Scenario: Tool component has licenses
- **WHEN** an SBOM is generated
- **THEN** the mesonsbom tool component SHALL contain a `licenses` array with `{"license": {"id": "GPL-3.0-or-later"}}`

### Requirement: Tool component includes external references
Each tool component in `tools.components` SHALL include an `externalReferences` array with the project homepage and VCS URLs.

#### Scenario: Tool component has external references
- **WHEN** an SBOM is generated
- **THEN** the mesonsbom tool component SHALL contain an `externalReferences` array
- **AND** one entry SHALL have `type` equal to `"website"` and `url` equal to `"https://github.com/Framstag/mesonsbom"`
- **AND** one entry SHALL have `type` equal to `"vcs"` and `url` equal to `"git+https://github.com/Framstag/mesonsbom.git"`

### Requirement: Tool component includes description
Each tool component in `tools.components` SHALL include a `description` field summarizing the tool's purpose.

#### Scenario: Tool component has description
- **WHEN** an SBOM is generated
- **THEN** the mesonsbom tool component SHALL have a `description` field set to `"CycloneDX SBOM generator from Meson build metadata"`

### Requirement: Tool component type
Each tool component SHALL have `type` set to `"application"`.

#### Scenario: Tool component type is application
- **WHEN** an SBOM is generated
- **THEN** every entry in `metadata.tools.components` SHALL have `type` equal to `"application"`
