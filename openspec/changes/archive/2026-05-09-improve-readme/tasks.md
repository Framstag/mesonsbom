## 1. README Content Updates

- [x] 1.1 Add disclaimer section at top of README.md after main title, stating mesonsbom is a personal project, not part of the official Meson project, developer not affiliated. (1)
- [x] 1.2 Expand Usage section with examples for all CLI options: `--build-dir`, `--target`, `--output`, `--version`, `--help`. Include a dedicated `--target` example showing target-specific SBOM generation. (2)
- [x] 1.3 Add License section to README.md stating GPL-3.0-or-later with SPDX identifier. (1)

## 2. AI Acknowledgement

- [x] 1.4 Add AI Acknowledgement section to README.md after FAQ, before License, crediting pi-agent, OpenSpec, and DeepSeek V4 via OpenRouter. (1)
- [x] 1.5 Add `pkgconf` to Required Build Dependencies list in README.md, with description of its purpose. (1)

## 4. Build System & Legal

- [x] 2.1 Add `license: 'GPL-3.0-or-later'` to the `project()` call in `meson.build`. (1)
- [x] 2.2 Create `LICENSE` file in repository root with full GPL-3.0 license text. (1)
- [x] 2.3 Add `project_author` variable to `meson.build` and include author name (Tim Teulings) in README License section. (1)