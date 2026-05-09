## 1. PkgConfigWrapper Enhancements

- [x] 1.1 Add `getPackageDescription` method to `PkgConfigWrapper` that retrieves the description from a `pkgconf_pkg_t`'s `pkg->description` field.
- [x] 1.2 Add `getPackageInfo` method that retrieves both name, version, and description in one call, returning a struct with these fields.

## 2. SBOMBuilder Modifications

- [x] 2.1 Extend `addComponent` signature to accept optional `description`, `purl`, and `licenses` parameters with empty defaults.
- [x] 2.2 Generate `purl` field using `pkg:generic/<name>@<version>` format for every component (lowercase name, percent-encoded special characters).
- [x] 2.3 Rewrite `addDependency` to consolidate all `to` refs into a single `dependsOn` array per `from` ref using a `std::unordered_map<std::string, std::vector<std::string>>`.
- [x] 2.4 Add `setTools` method to populate `metadata.tools` with the tool name (`mesonsbom`) and its version.
- [x] 2.5 Add `setLicenses` method to add license information to `metadata.component.licenses`.
- [x] 2.6 Update `writeTo` method to serialize consolidated dependencies array instead of individual entries.

## 3. Application Integration

- [x] 3.1 Update `loadDependencies` in `main.cpp` to pass through description for pkg-config resolved components (call `getPackageDescription` or `getPackageInfo`).
- [x] 3.2 Extract license array from `intro-projectinfo.json` and pass it to `SBOMBuilder::setLicenses`.
- [x] 3.3 Populate `metadata.tools` with `mesonsbom` identity after constructing the `SBOMBuilder`.
- [x] 3.4 Update dependency resolution loop to use the consolidated `addDependency` (ensure BFS loop calls `addDependency` only once per `from` component).

## 4. Testing & Verification

- [x] 4.1 Update existing tests for `addComponent` and `addDependency` signature changes.
- [x] 4.2 Verify consolidated `dependsOn` in integration tests (single entry per `ref` with multiple `to` items).
- [x] 4.3 Verify `purl` field format in generated SBOM matches `pkg:generic/<name>@<version>`.
- [x] 4.4 Verify `description` field is populated from `pkg-config` when available.
- [x] 4.5 Verify `metadata.tools` section contains `mesonsbom`.
- [x] 4.6 Verify license extraction from `intro-projectinfo.json`.
- [x] 4.7 Run the full test suite to ensure no regressions.
