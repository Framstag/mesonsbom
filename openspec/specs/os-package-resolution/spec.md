# OS Package Resolution

## Purpose

For each pkg-config-resolved dependency, resolve the OS package that provides it by querying the system package manager. When successful, the OS package identity takes precedence over the pkg-config identity.

## ADDED Requirements

### Requirement: OS package resolution for pkg-config dependencies
The system SHALL, for each dependency resolved via pkg-config, attempt to determine the OS package that owns the `.pc` file. The `.pc` file path SHALL be obtained from the `pkgconf_pkg_t.filename` field.

#### Scenario: Successful OS package resolution
- **WHEN** a dependency is resolved via pkg-config
- **AND** `pkgconf_pkg_t.filename` returns a path like `/usr/lib/pkgconfig/libcurl.pc`
- **AND** the OS package manager (e.g., `pacman -Qo`) confirms the file is owned by an OS package (e.g., `curl 8.20.0-5`)
- **THEN** the component SHALL use the OS package name as its primary `name`
- **AND** the component SHALL use the OS package version as its primary `version`
- **AND** the component `purl` SHALL use the format `pkg:generic/<os-pkg-name>@<os-version>?os=<pm>`
- **AND** the original pkg-config identity SHALL be preserved in `evidence.identity[]` and `properties`

#### Scenario: OS resolution fails, fallback to pkg-config
- **WHEN** a dependency is resolved via pkg-config
- **AND** the OS package manager query fails (file not owned, no PM detected, PM error)
- **THEN** the component SHALL use the pkg-config name and version as its primary identity
- **AND** the component `purl` SHALL use the existing format `pkg:generic/<name>@<version>` without qualifiers
- **AND** no `evidence` or `properties` block SHALL be added for OS package data
- **AND** the tool SHALL continue processing other dependencies without aborting

#### Scenario: No OS package manager detected
- **WHEN** the system runs on a platform without a recognized package manager (NixOS, Guix, container without PM)
- **THEN** all pkg-config dependencies SHALL fall back to pkg-config identity
- **AND** the tool SHALL NOT emit warnings about missing package manager (silent fallback)

### Requirement: Deduplication by OS package
When multiple pkg-config dependencies resolve to the same OS package, the SBOM SHALL contain only one component for that OS package.

#### Scenario: Two pkg-config deps map to same OS package
- **WHEN** dependency `libssl` resolves to OS package `openssl 3.0.14-1`
- **AND** dependency `libcrypto` also resolves to OS package `openssl 3.0.14-1`
- **THEN** the SBOM SHALL contain exactly one component with `name` = `"openssl"` and `version` = `"3.0.14-1"`
- **AND** the component SHALL include evidence from both pkg-config origins
- **AND** both `libssl` and `libcrypto` SHALL appear in the evidence identity array
- **AND** dependency edges from parent components SHALL reference the single `openssl` component

#### Scenario: Deduplication does not mix OS and pkg-config
- **WHEN** dependency `libfoo` resolves to OS package `foo`
- **AND** dependency `libbar` does NOT resolve to any OS package
- **THEN** `libfoo` component SHALL use OS identity
- **AND** `libbar` component SHALL use pkg-config identity
- **AND** no deduplication occurs between them

### Requirement: Cache of resolved OS packages
The system SHALL cache resolved OS packages by `.pc` file path to avoid redundant subprocess queries.

#### Scenario: Same .pc file queried twice
- **WHEN** the same `.pc` file path is queried for OS package ownership a second time
- **THEN** the cached result SHALL be returned without spawning a new subprocess