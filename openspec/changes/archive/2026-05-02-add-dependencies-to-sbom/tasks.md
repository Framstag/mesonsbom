## Implementation Tasks

- [x] **Parse intro‑dependencies.json**
  Update `src/main.cpp` to read the `intro‑dependencies.json` file, filter out optional dependencies, and add each resolved library as a CycloneDX component with its name and version.

- [x] **Add library components to SBOM**
  Use `SBOMBuilder::addComponent` to create a component of type `library` for each dependency and record a `dependsOn` relationship from the main application component.

- [x] **Update error handling**
  Ensure the program throws a clear `std::runtime_error` if `intro‑dependencies.json` cannot be opened, and propagate the error message to the user.

- [x] **Extend integration test**
  Modify `tests/integration_test.cpp` to verify that the generated `sbom.json` contains at least one component with `"type": "library"`.

- [x] **Run full test suite**
  Execute `meson test -C build` to ensure all unit and integration tests pass.

- [x] **Documentation update (optional)**
  Add a brief note in the README describing the new dependency extraction feature and any limitations regarding optional dependencies.

- [x] **Code cleanup**
  Run `clang-format` (or your preferred formatter) to keep the source tidy.
