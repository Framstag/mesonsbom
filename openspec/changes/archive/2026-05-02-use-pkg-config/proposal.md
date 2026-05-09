## Why

In the previous `list-transitive-dependencies` change, `libpkgconf` was introduced as a build dependency to avoid calling external shell tools. However, the implementation in `src/main.cpp` remained a placeholder. To fully enable transitive dependency resolution for system libraries that are not provided as Meson subprojects, the tool must actively use the `libpkgconf` API to query package requirements.

## What Changes

- **Encapsulation of `libpkgconf`**: Introduce a `PkgConfigWrapper` class to isolate the C-API of `libpkgconf` from the rest of the application logic.
- **Integration into Resolution Loop**: Update the `loadDependencies` helper in `src/main.cpp` to use the `PkgConfigWrapper` when Meson introspection files are unavailable for a given component.
- **Graceful Handling of Missing Packages**: If a dependency cannot be resolved via `libpkgconf` (i.e., it is not a pkg-config package), the system SHALL issue a warning to `stderr` and continue processing other dependencies.
- **Robust Error Handling**: Implement proper context management (setup/teardown) for `pkgconf_ctx_t` to prevent memory leaks.
- **Verification**: Add integration tests that simulate system libraries to verify that the library successfully retrieves transitive dependencies.

## Capabilities

### New Capabilities
- `system-dependency-resolution`: The ability to programmatically query `pkg-config` for the requirements/dependencies of external system libraries.

### Modified Capabilities
- `transitive-dependency-resolution`: Update the requirement to mandate the actual use of the `libpkgconf` API instead of a placeholder implementation.

## Impact

- **Code**: Creation of `src/pkg_config_wrapper.h` and `src/pkg_config_wrapper.cpp`.
- **Main Logic**: Modifications to `src/main.cpp` to replace the placeholder logic with the wrapper calls.
- **Build System**: No changes needed to `meson.build` as the dependency is already present.
