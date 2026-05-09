## MODIFIED Requirements

### Requirement: SBOM Generation from Meson Build Metadata
The system SHALL generate a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory. Each component in the generated SBOM MUST include a `purl` attribute, a `description` attribute (when available), and the project's license information MUST be extracted into the metadata section. The `metadata.tools` section MUST identify the generating tool using the tool's own embedded version. Dependency edges MUST be consolidated so that each component has a single `dependsOn` array listing all of its dependencies.

#### Scenario: Tool version shown as mesonsbom's own version
- **WHEN** the SBOM is generated
- **THEN** the `metadata.tools` section SHALL contain an entry with the tool name `"mesonsbom"` and the tool's own version string.
- **AND** the version SHALL be the version defined in the tool's build system, NOT the version of the project being analyzed.
- **AND** the tool component SHALL be of type `"application"`.

#### Scenario: Version query via `-v` or `--version`
- **WHEN** the user runs `mesonsbom --version` or `mesonsbom -v`
- **THEN** the tool SHALL print its version string to stdout and exit with code 0.
- **AND** the printed version SHALL match the version defined in the tool's build system.

#### Scenario: Version embedded at build time
- **WHEN** the tool is built via Meson
- **THEN** the version string from `project()` in `meson.build` SHALL be passed to the compiler as a preprocessor define.
- **AND** the define SHALL be the single source of truth for the tool's runtime version.
- **AND** the current version SHALL be set to "1.0.0".