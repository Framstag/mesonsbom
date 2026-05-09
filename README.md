# mesonsbom

> **Disclaimer:** This is a personal project. mesonsbom is **not** part of the official [Meson](https://mesonbuild.com) project. I, the developer, am not affiliated with or a member of the Meson project. This tool is provided as-is for convenience.

## Purpose

The `mesonsbom` tool generates a CycloneDX 1.6 SBOM in JSON format by reading Meson introspection JSON files from a specified build directory. It helps with supply‑chain security and compliance by providing a full list of direct dependencies.

## Required Build Dependencies

- `nlohmann_json` – header‑only JSON parser (used for parsing Meson introspection files).
- `cxxopts` – command‑line option parser.
- `catch2` – unit testing framework (used for integration tests).
- `pkgconf` – pkg-config implementation (used for resolving system library dependencies).

## Build Instructions

```bash
# Configure the build directory
meson setup build

# Compile the project
ninja -C build
```

Optional flags can be passed to `meson setup` (e.g., `-Ddebug=true`). To clean the build directory:

```bash
ninja -C build clean
```

## Usage

### Basic SBOM generation (full project)

```bash
# Generate an SBOM for the entire project in the current directory
./build/mesonsbom --build-dir <path-to-meson-build>

# Specify a custom output file
./build/mesonsbom --build-dir <path-to-meson-build> --output my-sbom.json
```

The command reads `meson-info/intro-projectinfo.json` and `meson-info/intro-dependencies.json` from the specified build directory and writes a valid CycloneDX 1.6 SBOM to `sbom.json` (or the specified output path).

### Target-specific SBOM

```bash
# Generate an SBOM for a single build target (e.g., a specific executable or library)
./build/mesonsbom --build-dir <path-to-meson-build> --target my_executable

# Combine with custom output
./build/mesonsbom --build-dir <path-to-meson-build> --target my_library --output target-sbom.json
```

The `--target` option filters the SBOM to only include the specified target and its direct dependencies. Useful when you need a focused SBOM for a specific output artifact.

### Other options

```bash
# Print version information
./build/mesonsbom --version

# Show help and exit
./build/mesonsbom --help
```

### Verification

```bash
# Verify the SBOM was created
test -f sbom.json && echo "SBOM generated successfully"
```

## FAQ

**Q:** *What if the `meson-info` directory is missing?*\
**A:** Ensure you run `meson introspect` or configure the project with Meson; the introspection files are generated in `meson-info` during the build.

**Q:** *How do I update the README when new dependencies are added?*\
**A:** Edit the **Required Build Dependencies** section to include the new libraries and re‑commit the changes.

## AI Acknowledgement

This project was created with assistance from AI tools:

- **[pi-agent](https://pi.dev/)** – coding agent framework driving the development loop.
- **[OpenSpec](https://github.com/Fission-AI/OpenSpec)** – structured change workflow (proposal, design, specs, tasks).
- **DeepSeek V4 (deepseek-v4-flash)** via **[OpenRouter](https://openrouter.ai)** – language model used for code generation and reasoning.

## License

Copyright © 2026 Tim Teulings

This project is licensed under the GNU General Public License v3.0 or later — see the [LICENSE](LICENSE) file for details.

SPDX-License-Identifier: `GPL-3.0-or-later`
