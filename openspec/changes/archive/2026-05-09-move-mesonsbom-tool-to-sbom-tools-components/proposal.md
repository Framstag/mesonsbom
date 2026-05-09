## Why

CycloneDX 1.5 and later deprecate the flat `metadata.tools` array in favor of `metadata.tools.components`, which uses full Component objects. This change is required to produce spec-compliant CycloneDX 1.6 output and unlocks richer tool metadata (description, supplier, external references, hashes, licenses). The spec currently uses the deprecated format, emitting only `name` and `version`.

## What Changes

- Remove deprecated flat `metadata.tools` array entirely
- Add `metadata.tools.components` (array of full Component objects) per CycloneDX 1.5+ schema
- Populate each tool component with all available information: `type`, `name`, `version`, `description`, `bom-ref`, `purl`, `supplier`, `licenses`, `externalReferences` (homepage, VCS)
- mesonsbom SHALL appear as the first entry in `tools.components`, even if additional tools are added in the future
- Replace `setTools()` with `addTool()` accepting structured tool metadata

## Capabilities

### New Capabilities

- `tool-metadata`: Rich tool component metadata in CycloneDX `metadata.tools.components` format with ordering guarantee

### Modified Capabilities

- `sbom-generation`: The existing `metadata.tools` requirement changes from flat array to `tools.components` Component array. Tool version test scenario updated accordingly.

## Impact

- **src/sbom_builder.h**: Replace `setTools(name, version)` with `addTool(ToolInfo)`. Internal storage changes from flat JSON array to `tools.components` object. Old flat `metadata.tools` array no longer emitted.
- **src/main.cpp**: Call site updated to pass additional metadata fields (description, supplier, homepage URL, VCS URL)
- **tests/**: Unit tests for `setTools()` and tool metadata output updated to match new format
- **README.md**: No changes needed (tool metadata is internal SBOM structure, not CLI-facing)