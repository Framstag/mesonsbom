## Why

The current README lacks a disclaimer about this being an unofficial personal project, contains no usage examples for `--target` or other CLI options, and does not state the project license. Users may confuse this tool with an official Meson project, struggle to use advanced features, and have no clarity on licensing terms.

## What Changes

- **Disclaimer section**: Add notice at the top that mesonsbom is a personal project, not affiliated with or endorsed by the Meson project.
- **Usage examples**: Add example invocations for all meaningful CLI options: `--build-dir`, `--target`, `--output`, `--version`, `--help`.
- **AI Acknowledgement section**: Add notice crediting pi-agent, OpenSpec, and DeepSeek V4 via OpenRouter for AI-assisted development.
- **Required Dependencies update**: Add `pkgconf` to the Required Build Dependencies list.
- **License and author section**: Add GPL-3.0-or-later as the project license with author attribution (Tim Teulings). State in README. Add `license` field to `meson.build` and `project_author` variable. Create a `LICENSE` file.
- **No breaking changes.**

## Capabilities

### New Capabilities
*(none)*

### Modified Capabilities
- `readme-creation`: Add disclaimer section, license section, and comprehensive CLI usage examples with target-specific invocation.

## Impact

- **README.md**: New disclaimer at top, expanded Usage section with `--target` and other options, new License section with author attribution (Tim Teulings), `pkgconf` added to dependency list.
- **meson.build**: Add `license: 'GPL-3.0-or-later'` to `project()` call and `project_author` variable.
- **LICENSE**: New file with GPL-3.0-or-later full text.
- No code changes. No API changes. No dependency changes.
