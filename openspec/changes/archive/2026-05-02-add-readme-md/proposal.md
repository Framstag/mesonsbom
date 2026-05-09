## Why

The project currently lacks a top‑level `README.md`. Without documentation, new contributors and users cannot discover how to build, install, or use the application, which hampers adoption and maintenance.

## What Changes

- Add a `README.md` file at the repository root.
- The README will be organized into clear chapters:
  - **Purpose** – brief description of what the application does.
  - **Required Build Dependencies** – list of Meson‑compatible libraries (e.g., `nlohmann_json`, `cxxopts`, `catch2`).
  - **Build Instructions** – step‑by‑step commands to configure and compile the project with Meson.
  - **Usage** – example command‑line usage of the `mesonsbom` binary.
  - **FAQ** – common questions and troubleshooting tips.

## Capabilities

### New Capabilities
- `readme-creation`: Introduces a new documentation capability. This creates a new spec at `specs/readme-creation/spec.md` describing the required content and structure of the README.

### Modified Capabilities
- *None* – this change only adds a new capability without altering any existing requirements.

## Impact

- **Code**: No source‑code changes; only a new `README.md` file is added to the repository root.
- **Tests**: No test changes required.
- **Documentation**: Users and contributors gain immediate guidance on building, running, and understanding the project.
