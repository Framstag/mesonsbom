# README Creation — Delta

## MODIFIED Requirements

### Requirement: README Creation
The system SHALL provide a top‑level `README.md` file in the repository root. The README must contain the following sections:
- **AI Acknowledgement** – credit for AI tools used during development: pi-agent, OpenSpec, and DeepSeek V4 via OpenRouter.
- **Disclaimer** – notice that mesonsbom is a personal project, not affiliated with or endorsed by the official Meson project.
- **License** – SPDX identifier and link to the full license text (GPL-3.0-or-later), with author attribution (Tim Teulings).
- **Purpose** – brief description of the application.
- **Required Build Dependencies** – list of required Meson dependencies (including `pkgconf`).
- **Build Instructions** – step‑by‑step commands to configure and compile the project.
- **Usage** – example invocations covering all meaningful CLI options: `--build-dir`, `--target`, `--output`, `--version`, `--help`.
- **FAQ** – common troubleshooting items.

#### Scenario: Successful README rendering
- **WHEN** a user opens the repository on GitHub or clones it locally,
- **THEN** the `README.md` displays the structured sections above in proper markdown format.

#### Scenario: Disclaimer shown before usage information
- **WHEN** a user opens the README,
- **THEN** the disclaimer SHALL appear at the top of the document, immediately after the main title.
- **AND** the disclaimer SHALL state that the project is a personal project, not part of the official Meson project, and that the developer is not affiliated with or a member of the Meson project.

#### Scenario: CLI usage includes --target example
- **WHEN** a user reads the Usage section,
- **THEN** the README SHALL include an example invocation with the `--target` option to generate an SBOM for a specific build target.
- **AND** the README SHALL include examples for `--build-dir`, `--output`, `--version`, and `--help`.

#### Scenario: AI Acknowledgement shown before License
P1 opens the README,
P2 README SHALL include an AI Acknowledgement section after FAQ and before License.
P3 AI Acknowledgement SHALL credit pi-agent, OpenSpec, and DeepSeek V4 via OpenRouter.

#### Scenario: License declared in README and meson.build
- **WHEN** a user views the repository,
- **THEN** the README SHALL include a License section stating GPL-3.0-or-later and naming the author (Tim Teulings) as copyright holder.
- **AND** the `meson.build` file SHALL contain `license: 'GPL-3.0-or-later'` in the `project()` call.
- **AND** the `meson.build` file SHALL declare a `project_author` variable set to the author name.
- **AND** a `LICENSE` file SHALL exist in the repository root with the full GPL-3.0 license text.
