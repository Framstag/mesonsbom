## Context

The repository currently has no top‑level `README.md`.  Users and contributors lack a single source of truth that explains what the project does, which dependencies are required, how to build it with Meson, how to run the generated `mesonsbom` binary, and answers to common questions.  Adding the README is a documentation‑only change that does not affect the code base or build process.

## Goals / Non‑Goals

**Goals:**
- Provide a clear, concise overview of the project's purpose.
- List all required build dependencies (e.g., `nlohmann_json`, `cxxopts`, `catch2`).
- Give step‑by‑step Meson build instructions.
- Show basic usage examples of the `mesonsbom` command‑line tool.
- Include an FAQ section with common troubleshooting items.

**Non‑Goals:**
- Modify any source code or build scripts.
- Add extensive API documentation (the README will focus on high‑level usage only).
- Provide exhaustive coverage of every possible Meson configuration option.

## Decisions

- **File location** – The `README.md` will live at the repository root (`/README.md`).
- **Format** – Plain Markdown with clearly marked headings for each chapter.
- **Content source** – Leverage existing `meson.build` data for dependencies and the `mesonsbom --help` output for usage examples.
- **Versioning** – No version number is needed in the README; it will be updated alongside future releases automatically.

## Risks / Trade‑offs

- **Risk:** The README may become outdated as dependencies or build steps change.
  **Mitigation:** Add a reminder in the project’s CI pipeline to lint the README for stale sections after each release.

- **Risk:** Over‑documentation could clutter the file.
  **Mitigation:** Keep each chapter concise (≤ 3‑4 paragraphs) and focus on essential information for new users.
