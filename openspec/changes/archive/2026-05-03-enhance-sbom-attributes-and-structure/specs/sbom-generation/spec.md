## MODIFIED Requirements

### Requirement: SBOM Generation from Meson Build Metadata
The system SHALL generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory. Each component in the generated SBOM MUST include a `purl` attribute, a `description` attribute (when available), and the project's license information MUST be extracted into the metadata section. The `metadata.tools` section MUST identify the generating tool. Dependency edges MUST be consolidated so that each component has a single `dependsOn` array listing all of its dependencies.

#### Scenario: Successful SBOM generation with purl and description
- **WHEN** the user runs `mesonsbom --build-dir /path/to/build`
- **THEN** the tool reads `projectinfo.json` and `dependencies.json` and writes a valid CycloneDX 1.6 SBOM to the specified output file.
- **AND** every component in the SBOM SHALL include a `purl` field using the format `pkg:generic/<name>@<version>`.
- **AND** every component that was resolved via `pkg-config` SHALL include a `description` field sourced from the `.pc` file's `Description:` field.
- **AND** components without a `pkg-config` description SHALL include an empty string for the `description` field.

#### Scenario: Consolidated dependency blocks
- **WHEN** component A depends on components B and C
- **THEN** the SBOM SHALL contain a single dependency entry `{"ref": "A", "dependsOn": ["B", "C"]}`.
- **AND** there MUST NOT be separate dependency entries for A→B and A→C.

#### Scenario: Tools section populated
- **WHEN** the SBOM is generated
- **THEN** the `metadata.tools` section SHALL contain an entry with the tool name `"mesonsbom"` and the tool's version.
- **AND** the tool component SHALL be of type `"application"`.

#### Scenario: Project license extraction
- **WHEN** `intro-projectinfo.json` contains a `license` array with meaningful values (not `"unknown"`)
- **THEN** the license information SHALL be added to `metadata.component.licenses` as CycloneDX license objects.
- **AND** if all license entries are `"unknown"`, the `licenses` array SHOULD be omitted.
