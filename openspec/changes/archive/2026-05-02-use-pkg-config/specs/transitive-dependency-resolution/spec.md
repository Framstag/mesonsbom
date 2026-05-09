## MODIFIED Requirements

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
