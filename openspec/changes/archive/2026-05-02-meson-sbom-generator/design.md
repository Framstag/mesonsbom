## Context

The project currently builds applications with Meson, which can output build metadata as JSON via `meson introspect`. There is no existing mechanism to generate a Software Bill of Materials (SBOM) from this information. The goal is to provide an automated way to produce a CycloneDX 1.6 SBOM for any Meson‑built binary, improving supply‑chain visibility.

## Goals / Non-Goals

**Goals:**
- Provide a C++ command‑line tool (`mesonsbom`) that reads Meson introspection JSON files from a specified build directory.
- Produce a CycloneDX 1.6 SBOM in JSON format covering the main application component and its runtime dependencies.
- Use Meson as the build system for the tool itself and be dynamically linked.
- **All features must have unit tests** – each functional unit should be covered by automated tests.


**Non-Goals:**
- Scanning source code or Meson build definitions directly.
- Extracting or validating license information (may be added later).
- Supporting other build systems (e.g., CMake, Bazel) in this iteration.

## Decisions

- **JSON parsing library:** Use the header‑only `nlohmann/json` library for simplicity and wide adoption.
- **SBOM generation:** Implement an internal `SBOMBuilder` class (based on `nlohmann::json`) to construct a CycloneDX 1.6 SBOM, avoiding external CycloneDX libraries.
- **Build system:** Use Meson for building the generator to keep tooling consistent with the target projects.
- **File discovery:** Expect `projectinfo.json` and `dependencies.json` to be located under `<buildDir>/meson-info/` (the default Meson introspection output directory). No recursive search is performed.

- **Testing library:** Use Catch2 (header‑only) for unit testing.

- **Error handling:** Use C++ exceptions for unrecoverable errors (e.g., missing JSON files) and return a non‑zero exit code.

## Risks / Trade-offs

- **Meson JSON format changes:** Future Meson releases might alter the structure of `projectinfo.json` or `dependencies.json`. Mitigation: validate required fields and log missing data.
- **Dynamic linking:** Requires the target environment to have compatible shared libraries; may complicate deployment on minimal containers.
- **Dependency version mismatches:** The chosen versions of `nlohmann/json` and the `SBOMBuilder` implementation must be compatible with the compiler and the target platform.

## Migration Plan

- Add the new binary to CI pipelines, generating `sbom.json` after a successful build.
- Verify the SBOM against known good outputs for a sample project.
- Document usage in the project's README.

## Open Questions

- Should the tool provide an option to embed the SBOM as an artifact in the build output directory automatically?
- Will downstream users need additional metadata (e.g., purl, supplier) that may be added in a future version?
