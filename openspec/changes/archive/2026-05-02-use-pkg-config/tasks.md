## 1. Core Library Wrapper

- [x] 1.1 Create `src/pkg_config_wrapper.h` defining the `PkgConfigWrapper` class with RAII for `pkgconf_ctx_t`.
- [x] 1.2 Implement `src/pkg_config_wrapper.cpp` to handle context initialization and `pkgconf_set_pkg_name`.
- [x] 1.3 Implement a method in `PkgConfigWrapper` to retrieve requirements (dependencies) for a given package name, returning a list of package names.

## 2. Application Integration

- [x] 2.1 Update `src/main.cpp` to include `pkg_config_wrapper.h`.
- [x] 2.2 Modify `loadDependencies` to instantiate `PkgConfigWrapper` and call its resolution method when Meson subproject data is unavailable.
- [x] 2.3 Implement the warning mechanism to notify the user via `stderr` if `PkgConfigWrapper` cannot resolve a dependency.

## 3. Testing & Verification

- [x] 3.1 Create a test case with a known system library (e.g., `zlib` or `glib-2.0`) to verify the wrapper retrieves its transitive dependencies.
- [x] 3.2 Create a test case with a non-existent package to verify the warning is issued and the process continues.
- [x] 3.3 Run the full test suite to ensure no regressions.
