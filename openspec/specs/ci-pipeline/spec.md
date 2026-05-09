# CI Pipeline for Meson Builds

## Purpose

The system shall provide a continuous integration workflow that builds the project and runs all tests on every push/PR/merge to the main branch.

## Requirements

### Requirement: CI build and test on push/PR/merge
The system SHALL provide a CI workflow that builds the project and runs all tests on every push to `main`, pull request targeting `main`, and merge to `main`.

#### Scenario: Successful CI run
- **WHEN** a commit is pushed to `main` or a pull request targets `main`
- **THEN** the CI workflow SHALL install all required build dependencies (`meson`, `ninja-build`, `nlohmann_json-dev`, `libcxxopts-dev`, `libpkgconf-dev`, `catch2`).
- **AND** the workflow SHALL configure the project with `meson setup builddir`.
- **AND** the workflow SHALL build the project with `ninja -C builddir`.
- **AND** the workflow SHALL run all tests with `meson test -C builddir`.
- **AND** the workflow SHALL report success only if all tests pass.

#### Scenario: CI failure on build error
- **WHEN** a commit introduces a build error
- **THEN** the CI workflow SHALL fail with a non-zero exit code.
- **AND** the workflow SHALL report the failure status on the commit/PR.

#### Scenario: CI failure on test failure
- **WHEN** a commit introduces a test failure
- **THEN** the CI workflow SHALL fail with a non-zero exit code.
- **AND** the workflow SHALL report the failure status on the commit/PR.
