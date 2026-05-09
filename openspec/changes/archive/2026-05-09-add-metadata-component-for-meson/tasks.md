## 1. Core Implementation

- [x] 1.1 Add read logic for `meson-info.json` in `main.cpp` after build-dir validation (design.md: Data flow)
- [x] 1.2 Call `sbom.addTool()` for Meson with version from `meson_version.full` and upstream metadata (design.md: ToolInfo for Meson)
- [x] 1.3 Add read logic for `intro-buildoptions.json` to detect backend name (design.md: ToolInfo for Backend)
- [x] 1.4 Call `sbom.addTool()` for backend tool with generic name-to-metadata mapping (design.md: ToolInfo for Backend)
- [x] 1.5 Add graceful error handling: warn on stderr for meson-info.json, silently skip backend if unknown

## 2. Testing

- [x] 2.1 Unit test: verify Meson tool appears as second entry in `tools.components` with correct version
- [x] 2.2 Unit test: verify backend tool (ninja) appears as third entry with name "ninja" and version "unknown"
- [x] 2.3 Unit test: verify missing `meson-info.json` does not crash and produces stderr warning
- [x] 2.4 Unit test: verify corrupt `meson-info.json` does not crash and produces stderr warning

## 3. Spec Update

- [x] 3.1 Create `specs/backend-tool-metadata/spec.md` for the new backend-tool-metadata capability
- [x] 3.2 Update `openspec/specs/tool-metadata/spec.md` to reflect ordering: mesonsbom → meson → backend

## 4. Build & Verify

- [x] 4.1 Build project, run all tests, verify output
- [x] 4.2 Generate SBOM and confirm all three tool entries present with correct ordering
