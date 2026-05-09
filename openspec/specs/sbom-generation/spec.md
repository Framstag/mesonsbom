# SBOM Generation from Meson Build Metadata

## Purpose

The system shall generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory.

## Requirements

### Requirement: SBOM Generation from Meson Build Metadata
When a component is resolved to an OS package, its identity (name, version, purl, supplier) SHALL come from the OS package. The original pkg-config identity SHALL be preserved in `evidence.identity[]` and `properties`.
The system SHALL generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory. Each component in the generated SBOM MUST include a `purl` attribute, a `description` attribute (when available), and the project's license information MUST be extracted into the metadata section. The `metadata.tools.components` section MUST identify the generating tool using full Component objects with name, version, type, description, supplier, licenses, and external references. Dependency edges MUST be consolidated so that each component has a single `dependsOn` array listing all of its dependencies. The top-level `serialNumber` field MUST be present and set to a valid `urn:uuid:<UUIDv4>` value. The `metadata.timestamp` field SHOULD be present and set to an ISO 8601 UTC timestamp (`YYYY-MM-DDTHH:mm:ssZ`) reflecting the moment of BOM construction.

#### Scenario: Successful SBOM generation with purl and description
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **THEN** the tool reads `projectinfo.json` and `dependencies.json` and writes a valid CycloneDX 1.6 SBOM to the specified output file.
- **AND** every component in the SBOM SHALL include a `purl` field
- **AND** every component that was resolved via `pkg-config` SHALL include a `description` field sourced from the `.pc` file's `Description:` field
- **AND** components without a `pkg-config` description SHALL include an empty string for the `description` field
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **THEN** the tool reads `projectinfo.json` and `dependencies.json` and writes a valid CycloneDX 1.6 SBOM to the specified output file.
- **AND** every component in the SBOM SHALL include a `purl` field using the format `pkg:generic/<name>@<version>`.
- **AND** every component that was resolved via `pkg-config` SHALL include a `description` field sourced from the `.pc` file's `Description:` field.
- **AND** components without a `pkg-config` description SHALL include an empty string for the `description` field.

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
- **THEN** the component SHALL include a `properties` array with `discovery:method`, `pkg-config:name`, `pkg-config:version`, `os:distribution`, `os:package`, and `os:package-version`

#### Scenario: Consolidated dependency blocks
- **WHEN** component A depends on components B and C
- **THEN** the SBOM SHALL contain a single dependency entry `{"ref": "A", "dependsOn": ["B", "C"]}`.
- **AND** there MUST NOT be separate dependency entries for A→B and A→C.

#### Scenario: Tool version shown as mesonsbom's own version
- **WHEN** an SBOM is generated
- **THEN** `metadata.tools.components` SHALL contain an entry with the tool name `"mesonsbom"`, the tool's own version string, type `"application"`, a `bom-ref` of `"tool-mesonsbom"`, and a `purl` of `"pkg:generic/mesonsbom@<version>"`
- **AND** the version SHALL be the version defined in the tool's build system, NOT the version of the project being analyzed
- **AND** the entry SHALL include `description`, `supplier`, `licenses`, and `externalReferences` as specified in the tool-metadata capability

#### Scenario: Version query via `-v` or `--version`
- **WHEN** the user runs `mesonsbom --version` or `mesonsbom -v`
- **THEN** the tool SHALL print its version string to stdout and exit with code 0.
- **AND** the printed version SHALL match the version defined in the tool's build system.

#### Scenario: Version embedded at build time
- **WHEN** the tool is built via Meson
- **THEN** the version string from `project()` in `meson.build` SHALL be passed to the compiler as a preprocessor define.
- **AND** the define SHALL be the single source of truth for the tool's runtime version.
- **AND** the current version SHALL be set to "1.0.0".

#### Scenario: Project license extraction
- **WHEN** `intro-projectinfo.json` contains a `license` array with meaningful values (not `"unknown"`)
- **THEN** the license information SHALL be added to `metadata.component.licenses` as CycloneDX license objects.
- **AND** if all license entries are `"unknown"`, the `licenses` array SHOULD be omitted.

#### Scenario: Supplier populated from /etc/os-release
- **WHEN** a component is resolved to an OS package
- **AND** `/etc/os-release` was successfully parsed with `NAME="Arch Linux"`
- **THEN** the component SHALL include `"supplier": {"name": "Arch Linux"}`
- **AND** if `/etc/os-release` is unavailable, the supplier field SHALL be omitted

#### Scenario: Deduplicated component has consolidated evidence
- **WHEN** two pkg-config dependencies (`libssl`, `libcrypto`) resolve to the same OS package `openssl`
- **THEN** the single `openssl` component SHALL include both `libssl` and `libcrypto` in its evidence
- **AND** both `.pc` file paths SHALL appear in `evidence.occurrences[]`

#### Scenario: Full-project SBOM filters deps by target references
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build` without `--target`
- **AND** `intro-targets.json` exists and lists dependencies per target
- **THEN** the SBOM SHALL only include dependencies referenced by at least one target
- **AND** deps discovered during Meson configuration but not used by any target SHALL be excluded
#### Scenario: Target-specific SBOM via --target option
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build --target my_executable`
- **THEN** the tool reads `intro-targets.json` and finds the target with `name` equal to `"my_executable"`.
- **AND** the main component in the SBOM SHALL use the target's `name` instead of the project name.
- **AND** the SBOM SHALL only include the dependencies listed in that target's `dependencies` field in `intro-targets.json`.
- **AND** other targets in the build directory SHALL NOT be included in the SBOM.

#### Scenario: Full-project SBOM when --target is omitted
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build` without `--target`
- **THEN** the tool SHALL generate an SBOM for the full project, using `intro-projectinfo.json` as before.
- **AND** the behavior SHALL be identical to the current full-project generation.

#### Scenario: SBOM includes serialNumber as URN UUID
- **WHEN** an SBOM is generated
- **THEN** the top-level `serialNumber` field SHALL be present.
- **AND** its value SHALL match the format `urn:uuid:<UUIDv4>`.
- **AND** the UUID portion SHALL be a valid UUIDv4 (xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx).
- **AND** each `SBOMBuilder` instance SHALL produce a different `serialNumber`.
- **AND** the same `SBOMBuilder` instance SHALL produce the same `serialNumber` each time `writeTo` is called.

#### Scenario: Unique seed per application start
- **WHEN** the application starts
- **THEN** the UUID generator SHALL seed its PRNG with a value that combines entropy from `std::random_device` and the current wall-clock time.
- **AND** two consecutive application starts SHALL produce different first-generated UUIDs (assuming at least 1 second apart).

#### Scenario: metadata.timestamp present and valid
- **WHEN** an SBOM is generated
- **THEN** the `metadata.timestamp` field SHALL be present.
- **AND** its value SHALL match the ISO 8601 UTC format `YYYY-MM-DDTHH:mm:ssZ`.
- **AND** the date/time SHALL be the current UTC time at the moment of BOM construction.

#### Scenario: Same instance produces stable timestamp
- **WHEN** an `SBOMBuilder` instance generates an SBOM
- **AND** `writeTo` is called multiple times on the same instance
- **THEN** the `metadata.timestamp` SHALL be identical across all calls.

#### Scenario: Different instances may have different timestamps
- **WHEN** two `SBOMBuilder` instances are created at different times
- **THEN** their `metadata.timestamp` values SHALL reflect their respective construction times.
