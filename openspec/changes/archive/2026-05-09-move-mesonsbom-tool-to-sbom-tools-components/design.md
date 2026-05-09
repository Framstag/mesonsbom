## Context

Currently `SBOMBuilder::setTools(name, version)` writes to the deprecated CycloneDX format:

```json
"metadata": {
  "tools": [{"name": "mesonsbom", "version": "1.1.0"}]
}
```

CycloneDX 1.5+ requires:

```json
"metadata": {
  "tools": {
    "components": [
      {
        "type": "application",
        "name": "mesonsbom",
        "version": "1.1.0",
        "bom-ref": "tool-mesonsbom",
        "purl": "pkg:generic/mesonsbom@1.1.0",
        "description": "CycloneDX SBOM generator from Meson build metadata",
        "supplier": {"name": "Tim Teulings"},
        "licenses": [{"license": {"id": "GPL-3.0-or-later"}}],
        "externalReferences": [
          {"type": "website", "url": "https://github.com/Framstag/mesonsbom"},
          {"type": "vcs", "url": "git+https://github.com/Framstag/mesonsbom.git"}
        ]
      }
    ]
  }
}
```

The change touches `sbom_builder.h` (API + data model) and `main.cpp` (call site).

## Goals / Non-Goals

**Goals:**
- Emit spec-compliant `metadata.tools.components` for CycloneDX 1.5+
- Expose all available tool metadata (description, supplier, license, homepage, VCS)
- Guarantee mesonsbom is always first in `tools.components` array
**Non-Goals:**
- Computing tool binary hashes at build time (complex, adds build-system coupling)
- Supporting multiple tools with ordering beyond "mesonsbom first"
- Breaking existing unit tests for unrelated features

## Decisions

### Decision 1: Structured tool metadata API

**Choice:** Replace `setTools(name, version)` with `addTool()` that accepts a structured `ToolInfo` struct and returns the index of the inserted tool.

```cpp
struct ToolInfo {
    std::string name;
    std::string version;
    std::string type = "application";
    std::string description;
    std::string supplierName;
    std::string homepageUrl;
    std::string vcsUrl;
    std::vector<std::string> licenses; // SPDX IDs
};

// Returns insertion order index (first call = 0, second = 1, ...)
size_t addTool(const ToolInfo& info);
```

**Rationale:**
- Named struct vs. positional params: self-documenting, extensible (new fields don't break callers)
- Returns index so callers can verify ordering if needed
- `type` defaults to `"application"` (common case for tools)
- Keeps `ToolInfo` lightweight — no optional wrappers, empty string = omit

**Alternative considered:** Individual setters (`setToolName()`, `setToolVersion()`, etc.) — discarded as verbose and error-prone (easy to forget required fields).

### Decision 2: Ordering guarantee via insertion-order vector

**Choice:** Store tool components in a `std::vector<ToolInfo>` and serialize in insertion order. `main.cpp` calls `addTool()` for mesonsbom first, ensuring it appears first in the output.

```cpp
// In SBOMBuilder:
std::vector<ToolInfo> toolComponents_;

size_t addTool(const ToolInfo& info) {
    toolComponents_.push_back(info);
    return toolComponents_.size() - 1; // index
}
```

**Rationale:**
- Simplest possible model: no sorting, no priority keys, no reordering logic
- Insertion order = output order — caller controls ordering by call sequence
- If future code adds more tools after mesonsbom, the first call still controls position

**Alternative considered:** Priority field on `ToolInfo` with sort — over-engineered for current needs. Can add later if ordering rules become complex.

**Alternative considered:** `std::list` / linked list — no advantage over vector for < 10 tools.

### Decision 3: Output format — `tools` changes from array to object

**Choice:** Write `metadata.tools` as an object with `components` array:

```cpp
void writeTools(nlohmann::json& bom) const {
    if (toolComponents_.empty()) return;
    nlohmann::json components = nlohmann::json::array();
    for (const auto& tool : toolComponents_) {
        components.push_back(serializeTool(tool));
    }
    bom["metadata"]["tools"]["components"] = components;
}
```

**Rationale:**
- Mirror the standard Component serialization logic already in `SBOMBuilder` (same fields: type, name, version, bom-ref, purl, description, licenses)
- Reuse existing `makePurl()` helper for generating `pkg:generic/mesonsbom@<version>`
- `bom-ref` generated as `"tool-<name>"` to avoid collisions with project component refs

**Alternative considered:** Storing as `nlohmann::json` directly — discarded because structured data enables validation and reuse.

### Decision 4: Tool purl generation

**Choice:** Use `pkg:generic/<lowercase-name>@<version>` — same format as project components.

```cpp
// Reuse existing makePurl:
comp["purl"] = makePurl(tool.name, tool.version);
```

**Rationale:**
- Consistent purl scheme across all components in the BOM
- No special tool-specific purl format needed

### Decision 5: External references as structured data

**Choice:** Accept homepage URL and VCS URL as separate fields in `ToolInfo`, map to CycloneDX `externalReferences` with types `"website"` and `"vcs"`.

**Rationale:**
- Cleaner than passing raw JSON arrays
- Open for extension (add more reference types later without API break)
- Empty URLs = omit the reference array entirely

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| `bom-ref` prefix `"tool-"` could collide with a component named "tool-something" | Unlikely in practice. If collision occurs, `addComponent` returns false (already added). We could add a collision check in `addTool`. |
| Supplier name "Tim Teulings" hardcoded in `main.cpp` | Acceptable — this is the sole author. If contributors join, change to organization name or read from `meson.build`. |
| No tool hash means SBOM consumers can't verify tool binary integrity | Hash computation adds build-system complexity (SHA-256 file digest at build time). Can be added later as an enhancement. |

## Migration Plan

1. Add `ToolInfo` struct and new `addTool()` + `writeTools()` to `sbom_builder.h`
2. Update `main.cpp` call site to pass full metadata
3. Update unit tests in `tests/test_json.cpp` to verify new format
4. Build, test, verify output matches expected CycloneDX 1.6 schema

Rollback: revert to old API — no data migration needed (tools metadata is ephemeral, generated fresh each run).

## Open Questions

- Should tool description be a longer paragraph vs. one-liner? The GitHub README description is "CycloneDX SBOM generator from Meson build metadata" — sufficient.
- Should we add `publisher` field alongside `supplier`? Not needed — supplier is sufficient per CycloneDX component spec.