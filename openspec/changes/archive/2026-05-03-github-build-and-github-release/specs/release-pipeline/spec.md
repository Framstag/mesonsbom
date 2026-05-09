## ADDED Requirements

### Requirement: Automated release on tag push
The system SHALL provide a release workflow that, when a Git tag matching `v*` is pushed, builds source archives, generates the project's own SBOM, creates a statically linked x86-64 binary, and publishes a GitHub Release with all artifacts.

#### Scenario: Successful release creation
- **WHEN** a Git tag matching `v*` (e.g., `v1.0.0`, `v2.1.0`) is pushed
- **THEN** the release workflow SHALL install all required build dependencies.
- **AND** the workflow SHALL create a source archive (`.tar.gz` and `.zip`) of the repository at the tagged commit.
- **AND** the workflow SHALL build the project with `meson setup builddir`.
- **AND** the workflow SHALL generate the project's own SBOM by running `./builddir/mesonsbom --build-dir builddir`.
- **AND** the workflow SHALL build a statically linked x86-64 binary of `mesonsbom` (e.g., via `-Dstatic=true` or equivalent flags).
- **AND** the workflow SHALL create a GitHub Release for the tag.
- **AND** the GitHub Release SHALL include the following artifacts:
  - Source archive (`.tar.gz`)
  - Source archive (`.zip`)
  - The generated SBOM (`sbom.json`)
  - The statically linked binary (`mesonsbom-x86_64-static` or equivalent).
- **AND** the workflow SHALL set `draft: false` and `prerelease: false` on the release.

#### Scenario: Release skipped on non-version tags
- **WHEN** a tag that does NOT match `v*` is pushed (e.g., `experimental`, `build-123`)
- **THEN** the release workflow SHALL NOT be triggered.
- **AND** no release SHALL be created.

#### Scenario: Static binary is executable and standalone
- **WHEN** the statically linked binary is downloaded from a release
- **THEN** it SHALL run without any dynamic library dependencies (verified via `ldd` or equivalent).
- **AND** it SHALL correctly generate an SBOM when invoked with `--build-dir <path>`.
