## Why

Currently, the `mesonsbom` tool only identifies and includes direct dependencies of the project. This provides an incomplete picture of the software supply chain. To ensure full security and compliance visibility, the tool must resolve and list transitive dependencies—dependencies of dependencies—providing a complete graph of all components involved in the final build.

## What Changes

- Implement a recursive dependency resolution mechanism to traverse the dependency tree.
- Integrate a `pkg-config` library to resolve transitive dependencies for system libraries without calling external shell tools.
- Update the SBOM generation logic to include all resolved transitive dependencies as components.
- Ensure that dependency edges in the generated SBOM correctly represent the relationship between components, including the transitive links.
- Add integration tests that utilize a nested dependency structure to verify the recursive resolution logic.

## Capabilities

### New Capabilities
- `transitive-dependency-resolution`: Logic to recursively parse Meson introspection files to identify all transitive dependencies.

### Modified Capabilities
- `dependency-extraction`: Update the extraction process to trigger the recursive resolution rather than stopping at the direct dependency list.

## Impact

- **Code**: Changes to `src/main.cpp` and `src/sbom_builder.h` to support graph traversal and storage of transitive components.
- **Tests**: Extension of `tests/integration_test.cpp` to include scenarios with multi-level dependency chains.
- **SBOM Output**: The resulting `sbom.json` will contain a more comprehensive list of components and dependency edges.
