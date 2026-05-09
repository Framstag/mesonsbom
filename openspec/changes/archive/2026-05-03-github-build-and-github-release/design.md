## Context

The project currently has a skeleton `.github/workflows/ci.yml` that triggers on push/PR, but it uses placeholder dependency installation commands and does not run tests. There is no release automation at all — releases must be created manually. With the project at version 1.0.0 and a working SBOM generator, automated CI and release pipelines are needed to support open-source distribution.

## Goals / Non-Goals

**Goals:**
- Replace the placeholder CI workflow with a fully functional one: real dependency installation, build, and `meson test`.
- Add a release workflow triggered by `v*` tags that creates source archives, generates the project's own SBOM, builds a statically linked x86-64 binary, and publishes a GitHub Release with all artifacts.

**Non-Goals:**
- Cross-platform CI (only x86-64 Linux — Ubuntu latest is sufficient).
- Code signing or notarization of release binaries.
- Publishing to package registries (Homebrew, APT, etc.).
- Container image builds.

## Decisions

### Decision 1: Two separate workflow files
The CI and release workflows are separate files (`.github/workflows/ci.yml` and `.github/workflows/release.yml`). This keeps triggers and permissions distinct — CI runs on every push/PR without needing release permissions, and the release workflow only runs on tags.

### Decision 2: Static build via GCC `-static` flag
For the static binary, use `gcc -static` during linking. In Meson this is achieved by adding `-static` to `link_args`. This is simpler than maintaining a separate static build definition. The output is named `mesonsbom-x86_64-static` (renamed post-build).

### Decision 3: Release management via `softprops/action-gh-release`
Use the `softprops/action-gh-release` GitHub Action (community standard) for creating releases and uploading artifacts. It handles draft/prerelease flags, artifact upload, and body text from a file or inline.

### Decision 4: Source archive via `actions/upload-artifact` + GitHub Archive
GitHub automatically provides source archives (`.tar.gz` and `.zip`) for every tag via its `/releases` endpoint. The release workflow can reference these directly — no need to manually create archives. The workflow just passes the auto-generated URLs in the release body, or alternatively uses `github.archive_url`.

*Alternative considered:* Manually creating archives with `git archive`. Rejected because GitHub's built-in archives are equivalent and require zero maintenance.

### Decision 5: SBOM generation uses the just-built mesonsbom
The release workflow builds mesonsbom first, then runs `./builddir/mesonsbom --build-dir builddir` to generate the project's own SBOM. This is a "dogfooding" approach — mesonsbom generates its own bill of materials.

### Decision 6: Dependency caching for CI performance
Use GitHub Actions cache for Meson build artifacts to speed up repeated runs. The cache key is based on the hash of `meson.build` and `meson.options` files.

### Decision 7: Separate CI and release Python steps for dep installation
Use `apt-get` to install system packages for both workflows. Dependencies: `meson`, `ninja-build`, `nlohmann_json-dev`, `libcxxopts-dev`, `libpkgconf-dev`, `catch2`.

## Risks / Trade-offs

- **[Risk] Static binary size**: A fully static binary with libstdc++ can be large (~5–10 MB). **Mitigation**: Use `-s` (strip) flag to reduce size. Document the expected binary size in the release notes.
- **[Risk] Static linking not available on all runners**: The `-static` flag requires glibc static libraries. **Mitigation**: Use `ubuntu-latest` which includes `libc6-dev` and `libstdc++-static` packages. Add explicit `sudo apt-get install libc6-dev` if needed.
- **[Risk] SBOM references build-time paths**: The SBOM may include build directory paths. **Mitigation**: Already handled — the SBOM generator uses relative names from introspection JSON.
- **[Risk] Release workflow permissions**: Creating a GitHub Release requires `contents: write` permission. **Mitigation**: Explicitly set `permissions:` in the release workflow YAML to avoid inheriting overly restrictive org defaults.
