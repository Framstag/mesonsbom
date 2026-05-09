## 1. Build System Changes

- [x] 1.1 Update version in `meson.build` from `'0.1.0'` to `'1.0.0'`.
- [x] 1.2 Add `cpp_args` define `-DMESONSBOM_VERSION` using `meson.project_version()` to the `mesonsbom` executable in `meson.build`.
- [x] 1.3 Add the same `MESONSBOM_VERSION` define to the test executable's `cpp_args` so tests can access it.

## 2. Application Changes

- [x] 2.1 Add `-v` / `--version` option to `cxxopts` in `src/main.cpp`.
- [x] 2.2 Implement `--version` handling: print `MESONSBOM_VERSION` to stdout and exit with code 0 before processing other options.
- [x] 2.3 Replace `projVersion` with `MESONSBOM_VERSION` in the `sbom.setTools("mesonsbom", ...)` call.

## 3. Testing & Verification

- [x] 3.1 Run `mesonsbom --version` to verify it prints `1.0.0` and exits with code 0.
- [x] 3.2 Run `mesonsbom -v` to verify the short flag works identically.
- [x] 3.3 Run the full SBOM generation on the build directory and verify `metadata.tools[0].version` is `"1.0.0"`.
- [x] 3.4 Run the full test suite to ensure no regressions.
