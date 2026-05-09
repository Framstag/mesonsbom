## ADDED Requirements

### Requirement: README Creation
The system SHALL provide a top‑level `README.md` file in the repository root. The README must contain the following sections:
- **Purpose** – brief description of the application.
- **Required Build Dependencies** – list of required Meson dependencies.
- **Build Instructions** – step‑by‑step commands to configure and compile the project.
- **Usage** – example invocation of `mesonsbom`.
- **FAQ** – common troubleshooting items.

#### Scenario: Successful README rendering
- **WHEN** a user opens the repository on GitHub or clones it locally,
- **THEN** the `README.md` displays the structured sections above in proper markdown format.
