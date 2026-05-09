# OS Release Detection

## Purpose

Detect the operating system distribution from the build host and identify the system package manager. This provides the supplier name and distribution identifier used when enriching SBOM components with OS package data.

## ADDED Requirements

### Requirement: Distribution detection from /etc/os-release
The system SHALL read `/etc/os-release` at startup to determine the operating system distribution.

#### Scenario: Successful parsing of /etc/os-release
- **WHEN** the tool starts
- **AND** `/etc/os-release` exists and contains `ID=arch` and `NAME="Arch Linux"`
- **THEN** the system SHALL store `id` = `"arch"` and `name` = `"Arch Linux"`
- **AND** the distribution name SHALL be used as the component supplier name when enriching with OS package data

#### Scenario: /etc/os-release missing
- **WHEN** the tool starts
- **AND** `/etc/os-release` does not exist or cannot be parsed
- **THEN** the system SHALL silently continue with empty distribution info
- **AND** OS package resolution SHALL still work (using PM detection alone)
- **AND** the supplier field SHALL NOT be set for OS-enriched components

### Requirement: Package manager detection
The system SHALL detect the system package manager at runtime by checking for known binaries in `$PATH`.

#### Scenario: Pacman detected
- **WHEN** the command `pacman` is found in `$PATH`
- **THEN** the system SHALL select `pacman` as the active package manager
- **AND** subsequent OS queries SHALL use `pacman -Qo <file>`

#### Scenario: Dpkg detected
- **WHEN** `pacman` is not found
- **AND** `dpkg-query` is found in `$PATH`
- **THEN** the system SHALL select `dpkg` as the active package manager
- **AND** subsequent OS queries SHALL use `dpkg -S <file>`

#### Scenario: RPM detected
- **WHEN** neither `pacman` nor `dpkg-query` are found
- **AND** `rpm` is found in `$PATH`
- **THEN** the system SHALL select `rpm` as the active package manager
- **AND** subsequent OS queries SHALL use `rpm -qf <file>`

#### Scenario: APK detected
- **WHEN** none of the above are found
- **AND** `apk` is found in `$PATH`
- **THEN** the system SHALL select `apk` as the active package manager
- **AND** subsequent OS queries SHALL use `apk info --who-owns <file>`

#### Scenario: No package manager detected
- **WHEN** none of `pacman`, `dpkg-query`, `rpm`, or `apk` are found in `$PATH`
- **THEN** the system SHALL report no package manager available
- **AND** OS package resolution SHALL fail gracefully (fallback to pkg-config identity)

### Requirement: Abstraction for future platform support
The detection logic SHALL be encapsulated in a dedicated class (`OsRelease`) that can be extended for non-Linux platforms.

#### Scenario: Platform detection boundary
- **WHEN** the Linux detection path is active
- **THEN** all detection logic SHALL be contained within `OsRelease` class
- **AND** no platform-specific code SHALL leak into `main.cpp` or `SBOMBuilder`