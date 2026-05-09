## Why

The generated SBOM currently lists `mesonsbom` itself as a tool in `metadata.tools.components` but omits the Meson build system that built the **analyzed project**. For a complete supply-chain transparency picture, the SBOM should identify the Meson tool and its exact version used to build the analyzed project, since the build tool is a critical part of the toolchain that produced the software artifacts.

## What Changes

- Read `meson_version` from `meson-info/meson-info.json` in the **analyzed project's build directory** (the `--build-dir` argument) to determine the Meson version that built that project
- Enrich that version with metadata from the Meson upstream project (homepage, VCS, license, description)
- Add a second entry in `metadata.tools.components` representing the Meson build system
- Build-directory information (`meson-info.json` from `--build-dir`) is the authoritative source for the version — NOT the Meson instance used to build mesonsbom itself
- Add a third entry in `metadata.tools.components` for the Meson backend build tool (e.g., ninja), detected generically from the `backend` option in `intro-buildoptions.json`
- Backend version defaults to "unknown" since the build directory does not contain the actual backend version — only the minimum required version is present in backend-specific files
- Use a generic detection framework: read backend name from `intro-buildoptions.json`, map to known upstream metadata (homepage, VCS, license) per backend type
- Keep `mesonsbom` as the first entry in `tools.components`; Meson SHALL appear as the second entry

## Capabilities

### New Capabilities
- `meson-tool-metadata`: Enrich the SBOM's `metadata.tools.components` with a CycloneDX Component object representing the Meson build system, carrying version from the build directory and upstream metadata (supplier, license, external references, description)
- `backend-tool-metadata`: Enrich the SBOM's `metadata.tools.components` with a CycloneDX Component object representing the Meson backend build tool (e.g., ninja, vs2022, xcode), detected generically from `intro-buildoptions.json` with known upstream metadata

### Modified Capabilities
- `tool-metadata`: The existing `mesonsbom-first` requirement remains unchanged. The spec SHALL be updated to reflect that Meson is the second tool entry
- `tool-metadata`: The spec SHALL be updated to reflect Meson as second entry and the backend tool as third entry

## Impact

- **src/main.cpp**: Read `meson-info.json` from the build directory, extract `meson_version.full`, and call `sbom.addTool()` with a `ToolInfo` for Meson
- **src/sbom_builder.h**: No API changes needed — `addTool()` already supports adding multiple tools
- **Tests**: New tests verifying Meson tool is present in `tools.components` as second entry with correct version and metadata
- **Build directory dependency**: The tool requires access to `meson-info.json` in the build directory (already accessed for other introspection files)
- **src/main.cpp**: Read `intro-buildoptions.json` from the build directory, extract `backend` option value, map to known metadata, and call `sbom.addTool()` for the backend tool
- **Build directory dependency**: The tool requires access to `intro-buildoptions.json` (already accessed for other introspection files)
