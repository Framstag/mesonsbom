## Why

The CycloneDX 1.6 specification requires every BOM document to include a `serialNumber` field — a globally unique identifier in `urn:uuid:<UUID>` format. Currently mesonsbom omits this field, producing technically invalid SBOMs that may be rejected by downstream consumers (scanners, aggregators, compliance tooling).

## What Changes

- **serialNumber field**: Every generated SBOM gets a top-level `serialNumber` field with a randomly-generated UUIDv4 in `urn:uuid:<UUID>` format.
- **UUID generation**: The UUID is generated once per SBOMBuilder instance (at construction time) using a system-provided random source. No external UUID library dependency — use a simple built-in UUIDv4 generator to avoid adding dependencies for a single small feature.

## Capabilities

### New Capabilities
*(none)*

### Modified Capabilities
- `sbom-generation`: Every generated CycloneDX 1.6 SBOM MUST include the `serialNumber` top-level field with a valid `urn:uuid:<UUIDv4>` value. The field SHALL be generated automatically — no user input or command-line flag needed.

## Impact

- **src/sbom_builder.h**: Constructor generates and stores a UUID. `writeTo` includes `serialNumber` in output JSON. A small UUIDv4 generation function added (private static or free function).
- **tests/**: New tests verifying `serialNumber` is present, starts with `urn:uuid:`, has valid UUID format, and each SBOMBuilder instance produces a different serial number.
- No new dependencies. No API/CLI changes. No breaking changes.