## Why

The currently generated CycloneDX SBOM is minimal — it contains component names, versions, and basic dependency edges, but lacks standard metadata fields that consumers expect. Adding descriptions, Package URLs (purl), a tools section, and license information makes the SBOM more useful for supply chain transparency, security scanning, and compliance tooling.

## What Changes

- **purl attribute**: Each component in the SBOM gets a `purl` field using the `pkg:generic/<name>@<version>` format.
- **description attribute**: Each dependency resolved via `pkg-config` includes its description (extracted from the `.pc` file). Dependencies from Meson introspection without a description get an empty string.
- **Consolidated dependsOn**: All dependencies of a single component are grouped into one `dependsOn` array per `ref`, instead of one entry per edge.
- **Tools section**: The SBOM's `metadata.tools` section is populated with information about `mesonsbom` (name, version).
- **License information**: The project's license is extracted from `intro-projectinfo.json` and added to `metadata.component.licenses`. Dependency license information is included if available from introspection or `pkg-config`.

## Capabilities

### New Capabilities
*(none — all changes are modifications to existing capabilities)*

### Modified Capabilities
- `sbom-generation`: Component attributes (purl, description), consolidated dependsOn blocks, tools section, metadata license extraction (from `intro-projectinfo.json`), dependency license extraction (from introspection or `pkg-config`).

## Impact

- **src/sbom_builder.h**: `addComponent` signature extended to accept optional description, purl, and license parameters. `addDependency` rewritten to consolidate dependsOn per ref. New methods `setTools`, `setMetadataLicense` for tools section and project license.
- **src/pkg_config_wrapper.h/cpp**: `getDependencies` extended to also return descriptions (or a new method `getPackageInfo` to retrieve name, version, description).
- **src/main.cpp**: Updated to pass extracted description, purl, and license data into `SBOMBuilder`. License read from `intro-projectinfo.json`. Tools section populated with `mesonsbom` identity.
- **tests/**: Existing tests updated for new `addComponent` and `addDependency` signatures. New tests cover purl generation, tools section, consolidated dependsOn, and license extraction.
