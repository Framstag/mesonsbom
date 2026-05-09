## Why

In multi-target Meson projects (e.g., a project with a library and a test executable), `mesonsbom` currently generates a single SBOM for the entire project using `intro-projectinfo.json`. This lumps all targets and their dependencies together. For complex projects, users need the ability to generate an SBOM for a **specific target** (e.g., only the library, not the tests).

## What Changes

- **Add an optional `-t` / `--target` CLI option**: Specify a target name from `intro-targets.json`. When provided:
  - Read `intro-targets.json` and find the matching target by `name`.
  - Use the target's `name` as the main component name (instead of the project name).
  - Only include dependencies listed in that target's `dependencies` field.
  - Exclude other targets and their unique dependencies.
- **When `--target` is not provided**: Behavior is unchanged — generates SBOM for the entire project.

## Capabilities

### New Capabilities
*(none)*

### Modified Capabilities
- `sbom-generation`: Add a `--target` CLI option for per-target SBOM generation. The existing full-project generation behavior remains the default.

## Impact

- **src/main.cpp**: Add `-t` / `--target` option to `cxxopts`. When set, load `intro-targets.json`, resolve the target, and use its name/dependencies instead of the full project scope.
- **Tests**: Add integration test verifying that `--target` filters output to only the specified target and its dependencies.
