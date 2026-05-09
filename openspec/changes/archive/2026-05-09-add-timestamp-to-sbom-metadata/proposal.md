## Why

The CycloneDX 1.6 specification recommends (SHOULD) including a `metadata.timestamp` field indicating when the BOM was generated, using ISO 8601 format. Currently mesonsbom omits this field, reducing SBOM utility for downstream consumers that rely on generation time for audit trails, freshness checks, and build reproducibility analysis.

## What Changes

- **metadata.timestamp field**: Every generated SBOM gets a `metadata.timestamp` field set to the current UTC date/time in ISO 8601 format (`YYYY-MM-DDTHH:mm:ssZ`).
- **Generation time**: The timestamp is captured once per SBOMBuilder instance at construction time, ensuring consistency across serialization calls.
- **No new dependencies**: Use C++ standard library time formatting (`std::time_t`, `std::gmtime`, `std::strftime`).

## Capabilities

### New Capabilities
*(none)*

### Modified Capabilities
- `sbom-generation`: The `metadata.timestamp` field SHALL be present in every generated SBOM, formatted as an ISO 8601 UTC timestamp reflecting the moment of BOM construction.

## Impact

- **src/sbom_builder.h**: Constructor captures current time and stores as ISO 8601 string. `writeTo` includes `metadata.timestamp` in output JSON. No API/CLI changes.
- **tests/**: New tests verifying `metadata.timestamp` presence, ISO 8601 format, same instance yields stable value, different instances may differ.
- No new dependencies. No breaking changes.
