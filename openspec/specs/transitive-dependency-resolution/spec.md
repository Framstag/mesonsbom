# transitive-dependency-resolution Specification

## Purpose
TBD - created by archiving change list-transitive-dependencies. Update Purpose after archive.
## Requirements
### Requirement: Recursive Dependency Resolution
The system SHALL recursively resolve all transitive dependencies by traversing the dependency graph starting from the project's direct dependencies. For every discovered dependency, the system MUST attempt to find its own dependencies within the build directory's introspection data or via a `pkg-config` library.

#### Scenario: Multi-level dependency chain
- **WHEN** the project depends on Library A, and Library A depends on Library B.
- **THEN** the generated SBOM includes both Library A and Library B as components.
- **AND** the dependency edges reflect the chain: Project -> Library A -> Library B.

#### Scenario: Circular dependency handling
- **WHEN** Library A depends on Library B, and Library B depends on Library A.
- **THEN** the system SHALL successfully terminate the resolution process without an infinite loop.
- **AND** both Library A and Library B are included in the SBOM exactly once.

### Requirement: Circular Dependency Notification
The system SHALL issue a warning to the user if a circular dependency is detected during the resolution process, but MUST continue the analysis to resolve all other remaining dependencies.

#### Scenario: Warning on cycle
- **WHEN** a circular dependency is detected (e.g., A -> B -> A).
- **THEN** the system prints a warning message to stderr indicating a cycle was found.
- **AND** the system continues to process other dependencies in the graph.

### Requirement: No External Tool Calls
The system MUST NOT invoke external shell commands (e.g., via `popen`, `system`, or similar) to retrieve dependency information. All data MUST be retrieved via direct file reads or through the use of a linked `pkg-config` library API.

#### Scenario: Programmatic pkg-config access
- **WHEN** the system needs to resolve dependencies for a system library.
- **THEN** it uses the `pkg-config` library API to retrieve the requirements.
- **AND** no external process is spawned for this operation.

