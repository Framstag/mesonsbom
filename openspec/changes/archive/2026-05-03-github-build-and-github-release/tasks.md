## 1. CI Pipeline (spec: ci-pipeline)

- [x] 1.1 Rewrite `.github/workflows/ci.yml`: add real dependency installation (`meson`, `ninja-build`, `nlohmann_json-dev`, `libcxxopts-dev`, `libpkgconf-dev`, `catch2`).
- [x] 1.2 Add Meson build cache step using `actions/cache` keyed on `meson.build` hash.
- [x] 1.3 Add `meson setup builddir`, `ninja -C builddir`, and `meson test -C builddir` steps.
- [x] 1.4 Remove placeholder dependency comment and `echo` steps from the workflow.

## 2. Release Pipeline (spec: release-pipeline)

- [x] 2.1 Create `.github/workflows/release.yml` with trigger: `push: tags: ['v*']` and explicit `permissions: contents: write`.
- [x] 2.2 Add dependency installation and build steps (same as CI).
- [x] 2.3 Add step to build statically linked binary: use `-Dstatic=true` or pass `-static` in `link_args`.
- [x] 2.4 Add step to run `mesonsbom` on itself to generate the project's own SBOM (`./builddir/mesonsbom --build-dir builddir`).
- [x] 2.5 Rename the static binary to `mesonsbom-x86_64-static` and prepare artifacts for upload.
- [x] 2.6 Add `softprops/action-gh-release` step to create GitHub Release with all artifacts (source archives, sbom.json, static binary).

## 3. Verification

- [x] 3.1 Verify `ci.yml` parses correctly: `act --lint` or `yamlint` on the workflow file.
- [x] 3.2 Verify `release.yml` parses correctly by inspection and YAML linting.
- [x] 3.3 Run `meson setup build && ninja -C build && meson test -C build` locally to confirm build and test still pass.
