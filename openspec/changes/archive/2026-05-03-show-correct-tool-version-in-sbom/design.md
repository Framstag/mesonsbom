## Context

The current SBOM generator (`mesonsbom`) has a hard-coded version `'0.1.0'` in `meson.build` via the `project()` call. However, this version is never made available to the C++ application at runtime. Instead, the SBOM's `metadata.tools` entry for `mesonsbom` incorrectly reuses `projVersion` — the version of the *project being analyzed*. This means the tool version in the SBOM changes depending on what project is being scanned, which is wrong.

Additionally, there is no `--version` flag to let users query the tool version directly.

## Goals / Non-Goals

**Goals:**
- Make `mesonsbom`'s own version available to the C++ application at compile time.
- Add a `-v` / `--version` CLI flag that prints the version and exits with code 0.
- Fix the SBOM's `metadata.tools` entry to use `mesonsbom`'s own version instead of the analyzed project's version.
- Keep the version source of truth in `meson.build` (`project()` version string).

**Non-Goals:**
- Changing how third-party dependency versions are determined (those stay from introspection data).
- Adding version info to `metadata.component` (the project license/name changes are separate).
- Any changes to the CycloneDX spec version or BOM format structure.

## Decisions

### Decision 1: Embed version via Meson `cpp_args`
The cleanest way to propagate the Meson project version to C++ code is through a preprocessor define in `meson.build`:

```meson
meson_version_arg = '-DMESONSBOM_VERSION="' + meson.project_version() + '"'

executable('mesonsbom', src,
  dependencies : [...],
  cpp_args: meson_version_arg,
  ...)
```

Alternatives considered:
- **Generate a version header at build time** via `configure_file()`: More complex, requires a header template and build-time file generation. Overkill for a single string.
- **Link-time version string**: Not applicable for a single executable.
- **Read own `intro-projectinfo.json`**: Fragile — requires the binary to locate its own build directory at runtime.

**Decision**: Use `cpp_args` with `-DMESONSBOM_VERSION`. Simple, reliable, and keeps the version source of truth in `meson.build`.

### Decision 2: `--version` vs `-v`
Use `cxxopts`'s standard boolean option pattern, supporting both short and long forms for convenience:
```cpp
("v,version", "Print version and exit")
```

### Decision 3: SBOM tool version source
Replace `projVersion` with the `MESONSBOM_VERSION` define in the `setTools()` call:
```cpp
// Before (wrong): sbom.setTools("mesonsbom", projVersion);
// After (correct): sbom.setTools("mesonsbom", MESONSBOM_VERSION);
```

## Risks / Trade-offs

- **[Risk] Build tool coupling**: If someone builds `mesonsbom` outside of Meson (e.g., direct `g++` invocation), the `MESONSBOM_VERSION` define will be missing, leading to a compilation error. **Mitigation**: Acceptable — Meson is the intended build system and documented as such.
- **[Risk] Stale version**: The version in `meson.build` may not be updated before release. **Mitigation**: Standard semantic versioning practice; no different from any other Meson project.
- **[Risk] Escape issues with the define string**: If the version contains special characters, the preprocessor stringification may break. **Mitigation**: Version strings follow strict semver (`X.Y.Z`), which has no special characters.
