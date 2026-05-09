## Why

The SBOM currently identifies pkg-config-resolved dependencies only by their pkg-config name and version (e.g., "libcurl 7.88.1"). On OS-managed systems, these libraries are provided by OS packages (e.g., Arch pacman package "curl 8.20.0-5"). Without OS package identity, SBOM consumers cannot map components to OS vulnerability advisories (Arch CVE tracker, Debian Security Tracker, etc.) or verify package provenance. OS package identity fills this gap, making the SBOM actionable for system-level supply chain security.

## What Changes

- The tool SHALL, for each pkg-config-resolved dependency, attempt to determine the OS package that provides it by querying the system package manager
- When an OS package is identified, its name and version SHALL take precedence over the pkg-config identity for the primary component fields (name, version, purl)
- The original pkg-config identification data SHALL be preserved in component evidence and properties
- When OS package resolution fails (no package manager, file not owned, etc.), the tool SHALL fall back to the existing pkg-config-only identity with no change in behavior
- The resolution SHALL be implemented as a two-phase pipeline: collect all raw dependencies first, resolve OS packages, deduplicate by OS package, then build the SBOM
- A new OS detection module SHALL be introduced, abstracted for future platform support (Windows, Apple)
- The tool reads `/etc/os-release` for distribution identification and supplier metadata

## Capabilities

### New Capabilities
- `os-package-resolution`: Resolve the OS package name and version for each pkg-config-resolved dependency by querying the system package manager (pacman, dpkg, rpm, apk, etc.)
- `os-release-detection`: Detect the operating system distribution from `/etc/os-release` and identify the system package manager

### Modified Capabilities
- `sbom-generation`: The purl format extends with OS package qualifier when OS identity is known (`pkg:generic/<name>@<version>?os=<pm>`). Component identity (name, version, supplier) SHALL come from the OS package when resolved. The `evidence` field SHALL be populated with the discovery chain (pkg-config origin, OS package query). Component properties SHALL carry both pkg-config and OS package metadata.
- `dependency-extraction`: Dependencies SHALL be collected in a raw phase first, before OS resolution and deduplication. The component creation step SHALL use the resolved identity rather than raw pkg-config data.
- `system-dependency-resolution`: Resolution flow SHALL pass through deduplication and OS package enrichment before components are finalized.

## Impact

- **src/main.cpp**: Restructured into two-phase pipeline (collect → resolve → deduplicate → build)
- **src/pkg_config_wrapper.h/.cpp**: Add `getPackageFilename()` accessor exposing the `.pc` file path
- **src/os_release.h/.cpp** (new): Parse `/etc/os-release` for distribution ID and name
- **src/os_package_resolver.h/.cpp** (new): Detect package manager, query owning OS package, cache results
- **src/sbom_builder.h**: Add methods for `addEvidence()`, `addOccurrence()`, and component properties
- **tests/**: Unit tests for OS package resolution, deduplication, evidence output, fallback behavior
- **README.md**: No changes needed (CLI interface unchanged)