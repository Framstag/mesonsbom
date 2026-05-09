# SBOM Generation from Meson Build Metadata — Delta

## ADDED Requirements

### Requirement: SBOM generation timestamp in metadata
The system SHALL include a `metadata.timestamp` field in every generated CycloneDX SBOM, formatted as an ISO 8601 UTC timestamp (`YYYY-MM-DDTHH:mm:ssZ`). The timestamp SHALL reflect the moment of BOM construction.

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
