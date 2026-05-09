## 1. Core Logic Implementation

- [x] 1.1 Update `SBOMBuilder` to maintain a set of visited components to prevent infinite loops and duplicate entries.
- [x] 1.2 Implement the recursive traversal mechanism (BFS/DFS) to explore dependencies of identified components.
- [x] 1.3 Update `src/main.cpp` to trigger the recursive resolution process after extracting direct dependencies.
- [x] 1.4 Integrate a `pkg-config` library to retrieve transitive dependencies for system libraries (no external shell calls).
- [x] 1.5 Implement the warning mechanism to notify the user when a circular dependency is detected.

## 2. SBOM Graph Integration

- [x] 2.1 Ensure every resolved transitive dependency is added as a component with correct `bom-ref`, `name`, and `version`.
- [x] 2.2 Update the dependency edge creation logic to link transitive dependencies to their respective parents.
- [x] 2.3 Verify that the final `sbom.json` output adheres to the CycloneDX 1.6 JSON schema.

## 3. Testing & Verification

- [x] 3.1 Create a test environment with a nested dependency chain (e.g., Project -> A -> B -> C).
- [x] 3.2 Add integration tests to verify that all components in the chain are present in the SBOM.
- [x] 3.3 Add a test case with circular dependencies to verify the loop termination and the circularity warning.
- [x] 3.4 Run all existing and new tests to ensure no regressions in basic SBOM generation.

## 4. Review & Finalisation

- [x] 4.1 Perform a final review of the generated SBOM with a complex dependency tree.
- [x] 4.2 Verify that warnings are correctly printed to `stderr` as specified in the requirements.
