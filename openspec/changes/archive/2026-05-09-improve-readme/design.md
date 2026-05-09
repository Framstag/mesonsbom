## Context

Current README provides basic build instructions and a single usage example (`mesonsbom --build-dir`). No disclaimer about project status exists. No license is stated. The `--target` option, `--output`, `--version`, and `--help` flags are undocumented.

## Goals / Non-Goals

**Goals:**
- Add disclaimer: personal project, not official Meson or affiliated
- Add `pkgconf` to Required Build Dependencies
- Document all CLI options with examples: `--build-dir`, `--target`, `--output`, `--version`, `--help`
- Declare GPL-3.0-or-later license and author in README and meson.build
- Create LICENSE file

**Non-Goals:**
- Code changes
- Functional changes to the tool
- Translation or multi-language docs
- CI/CD documentation changes

### 4. AI Acknowledgement: new section after FAQ, before License

**Decision**: Add an "AI Acknowledgement" section between FAQ and License listing pi-agent, OpenSpec, and DeepSeek V4 via OpenRouter.

**Rationale**: Transparent disclosure of AI tooling used during development. Placement after FAQ keeps it near the end but before legal text.

## Decisions

### 1. No design doc needed — pure content change

**Decision**: This change is documentation-only. No architectural decisions, no code design. The design doc is minimal by nature.

**Rationale**: README edits, meson.build metadata field, and LICENSE file creation require no technical design — only content decisions.

### 2. License and author: GPL-3.0-or-later with Tim Teulings

**Decision**: Use GPL-3.0-or-later with the SPDX identifier. Add `license: 'GPL-3.0-or-later'` to `meson.build`. Add `project_author` variable to `meson.build`. Include author name in README License section.

**Rationale**: Standard Meson license declaration format. The `project_author` variable documents the author in the build system for downstream tooling.

### 3. Disclaimer placement: top of README

**Decision**: Place disclaimer immediately after the main title, before Purpose.

**Rationale**: Users see it first. Avoids confusion before they read further.

## Risks / Trade-offs

- **[Risk] Users skip the disclaimer**: No mitigation beyond prominent placement.
- **[Risk] License file verbatim**: GPL-3.0-or-later has a standard full text. Use the canonical GPL-3.0 text from gnu.org to ensure legal correctness.
