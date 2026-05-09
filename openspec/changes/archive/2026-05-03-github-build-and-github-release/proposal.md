## Why

The project currently has a skeleton CI workflow (`.github/workflows/ci.yml`) that runs basic build steps but is incomplete — dependencies are listed as placeholders and there is no release automation. For a mature open-source tool, automated CI on every commit/PR/merge and automated GitHub Releases with source archives, the SBOM, and a statically-linked binary are essential.

## What Changes

- **Replace the existing `ci.yml`** with a fully functional CI workflow:
  - Trigger on push, pull request, and merge to `main`.
  - Install all real dependencies (Meson, Ninja, `nlohmann_json`, `cxxopts`, `pkgconf`, etc.).
  - Build the project and run all tests (`meson test`).
- **Add a new release workflow** (or extend CI) that:
  - Triggers on Git tag push (e.g., `v*`).
  - Builds sources and creates a source archive (`.tar.gz`/`.zip`).
  - Generates the SBOM for the mesonsbom project itself.
  - Creates a **statically linked x86-64 binary** of `mesonsbom`.
  - Publishes a GitHub Release with all artifacts.
- **Upgrade the existing workflow** from placeholder deps to real installation.

## Capabilities

### New Capabilities
- `ci-pipeline`: CI workflow for continuous integration (build + test on push/PR/merge).
- `release-pipeline`: Release workflow (source archives, SBOM publishing, static binary, GitHub Release).

### Modified Capabilities
*(none)*

## Impact

- `.github/workflows/ci.yml`: Complete rewrite — real dependency installation, proper build/test steps, remove placeholder.
- `.github/workflows/release.yml`: New file for tag-triggered release workflow.
- **Static build**: May require adding a static build configuration or CMake flag for `-static`.
- No application code changes needed.
