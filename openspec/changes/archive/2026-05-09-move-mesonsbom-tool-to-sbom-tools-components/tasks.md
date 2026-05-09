## 1. Data model and API in sbom_builder.h

- [x] 1.1 Add `ToolInfo` struct with fields: name, version, type (default "application"), description, supplierName, homepageUrl, vcsUrl, licenses vector
- [x] 1.2 Add `std::vector<ToolInfo> toolComponents_` member to `SBOMBuilder`
- [x] 1.3 Remove old `setTools(name, version)` method
- [x] 1.4 Add `addTool(const ToolInfo& info) -> size_t` that appends to `toolComponents_` and returns insertion index
- [x] 1.5 Add private `serializeTool(const ToolInfo& tool) -> nlohmann::json` that builds a full Component object with: type, name, version, bom-ref ("tool-<name>"), purl (via makePurl), description, supplier, licenses, externalReferences (website + vcs)
- [x] 1.6 Update `writeTo()` to call a new `writeTools()` helper that writes `metadata.tools` as `{"components": [...]}` object, replacing the old flat array entirely

## 2. Call site update in main.cpp

- [x] 2.1 Replace `sbom.setTools("mesonsbom", MESONSBOM_VERSION)` with `sbom.addTool({...})` with full metadata

## 3. Unit tests

- [x] 3.1 Update `test_json.cpp` `"SBOMBuilder setTools and setLicenses"` test to use `addTool()` and check new `tools.components` object structure
- [x] 3.2 Update `integration_test.cpp` tool metadata check from `tools[n]` array to `tools.components[n]` object path
- [x] 3.3 Add test: "Tool ordering - mesonsbom is always first entry" that adds two tools and verifies mesonsbom is at index 0
- [x] 3.4 Add test: "Tool component has all required fields" that verifies bom-ref = "tool-mesonsbom", purl format, supplier name, licenses array, externalReferences with types "website" and "vcs"

## 4. Build and verify

- [x] 4.1 Rebuild project with `ninja -C build`
- [x] 4.2 Run all tests (13/13 sbom tests pass, 75 assertions)
- [x] 4.3 Generate SBOM and manually inspect `metadata.tools.components` structure in output