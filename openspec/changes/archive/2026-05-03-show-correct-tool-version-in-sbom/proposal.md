## Why

The generated SBOM currently lists `mesonsbom` as the generating tool, but the version shown is the version of the *project being analyzed* — not `mesonsbom`'s own version. When analyzing an external project, the tool version becomes that project's version (e.g., `"3.14.0"` for a project depending on Catch2), which is incorrect and misleading. The tool must know its own version, expose it via a command-line flag, and use it consistently in the SBOM.

## What Changes

- **Embed tool version at build time**: Meson's `project()` version string (`'0.1.0'`) will be passed to the C++ compiler as a preprocessor define (e.g., `-DMESONSBOM_VERSION='"0.1.0"'`).
- **Add `-v` / `--version` CLI option**: Calling `mesonsbom --version` prints the tool's version and exits.
- **Fix tool version in SBOM**: The `metadata.tools` entry for `mesonsbom` uses the embedded version instead of the analyzed project's version.

## Capabilities

### New Capabilities
*(none)*

### Modified Capabilities
- `sbom-generation`: Tool version source changed from analyzed project version to embedded build-time version. New requirement for `--version` CLI flag.

## Impact

- **meson.build**: Add a `-DMESONSBOM_VERSION` define to the executable's `cpp_args` using `meson.project_version()`.
- **src/main.cpp**: Add `-v` / `--version` option to `cxxopts`. Use `MESONSBOM_VERSION` define in `setTools()` call.
- **tests/**: Update integration tests to verify tool version in SBOM matches expected value. Add test for `--version` flag.
