# SBOM Generation from Meson Build Metadata

## Purpose

The system shall generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory. When OS package data is available, the SBOM SHALL use OS identity for components and include evidence and properties documenting the discovery chain.

## MODIFIED Requirements

### Requirement: SBOM Generation from Meson Build Metadata
The system SHALL generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory. Each component in the generated SBOM MUST include a `purl` attribute, a `description` attribute (when available), and the project's license information MUST be extracted into the metadata section. The `metadata.tools.components` section MUST identify the generating tool using full Component objects with name, version, type, description, supplier, licenses, and external references. Dependency edges MUST be consolidated so that each component has a single `dependsOn` array listing all of its dependencies. The top-level `serialNumber` field MUST be present and set to a valid `urn:uuid:<UUIDv4>` value. The `metadata.timestamp` field SHOULD be present and set to an ISO 8601 UTC timestamp (`YYYY-MM-DDTHH:mm:ssZ`) reflecting the moment of BOM construction.

When a component is resolved to an OS package, its identity (name, version, purl, supplier) SHALL come from the OS package. The original pkg-config identity SHALL be preserved in `evidence.identity[]` and `properties`.

#### Scenario: Successful SBOM generation with purl and description
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **THEN** the tool reads `projectinfo.json` and `dependencies.json` and writes a valid CycloneDX 1.6 SBOM to the specified output file.
- **AND** every component in the SBOM SHALL include a `purl` field
- **AND** every component that was resolved via `pkg-config` SHALL include a `description` field sourced from the `.pc` file's `Description:` field
- **AND** components without a `pkg-config` description SHALL include an empty string for the `description` field

#### Scenario: OS package identity purl format with qualifier
- **WHEN** a component is resolved to an OS package from `pacman`
- **AND** the OS package name is `curl` and version is `8.20.0-5`
- **THEN** the component `purl` SHALL be `pkg:generic/curl@8.20.0-5?os=pacman`
- **AND** no additional purl qualifiers SHALL be present

#### Scenario: pkg-config-only purl unchanged
- **WHEN** a dependency is NOT resolved to an OS package
- **THEN** the component `purl` SHALL be `pkg:generic/<name>@<version>` (existing format, unchanged)

#### Scenario: Evidence block present for OS-resolved components
- **WHEN** a component has been enriched with OS package data
- **THEN** the component SHALL include an `evidence` object
- **AND** `evidence.identity` SHALL be an array with at least two entries
- **AND** one identity entry SHALL have `field` set to `"name"` and `concludedValue` set to the pkg-config name
- **AND** one identity entry SHALL have `field` set to `"purl"` and `concludedValue` set to the OS package purl
- **AND** each identity entry SHALL have `confidence` set to `1.0`
- **AND** each identity entry SHALL have at least one `method` with `technique` set to `"manifest-analysis"` and `confidence` set to `1.0`
- **AND** `evidence.occurrences` SHALL contain an entry with `location` set to the `.pc` file path

#### Scenario: Properties for OS-resolved components
- **WHEN** a component has been enriched with OS package data
- **THEN** the component SHALL include a `properties` array with:
  - `{"name": "discovery:method", "value": "pkg-config"}`
  - `{"name": "pkg-config:name", "value": "<pkg-config dependency name>"}`
  - `{"name": "pkg-config:version", "value": "<pkg-config version>"}`
  - `{"name": "os:distribution", "value": "<os-release ID>"}`
  - `{"name": "os:package", "value": "<OS package name>"}`
  - `{"name": "os:package-version", "value": "<OS package version>"}`

#### Scenario: Supplier populated from /etc/os-release
- **WHEN** a component is resolved to an OS package
- **AND** `/etc/os-release` was successfully parsed with `NAME="Arch Linux"`
- **THEN** the component SHALL include `"supplier": {"name": "Arch Linux"}`
- **AND** if `/etc/os-release` is unavailable, the supplier field SHALL be omitted

#### Scenario: Deduplicated component has consolidated evidence
- **WHEN** two pkg-config dependencies (`libssl`, `libcrypto`) resolve to the same OS package `openssl`
- **THEN** the single `openssl` component SHALL include both `libssl` and `libcrypto` in its evidence
- **AND** both `.pc` file paths SHALL appear in `evidence.occurrences[]`

### Requirement: Consolidated dependency blocks  
- **WHEN** component A depends on components B and C
- **THEN** the SBOM SHALL contain a single dependency entry `{"ref": "A", "dependsOn": ["B", "C"]}`.
- **AND** there MUST NOT be separate dependency entries for A→B and A→C.