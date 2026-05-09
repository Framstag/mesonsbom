## Context

The SBOM generator already reads several introspection files from the analyzed project's `meson-info/` directory (`intro-projectinfo.json`, `intro-dependencies.json`, `intro-targets.json`). One file it currently ignores is `meson-info.json` — the top-level Meson metadata file that includes the exact Meson version used to configure/build the analyzed project.

`SBOMBuilder::addTool()` already supports adding multiple tools via `ToolInfo` struct and insertion-order vector. No API changes needed in `sbom_builder.h` — only a new call site in `main.cpp`.

From the Meson upstream project (https://github.com/mesonbuild/meson):
- License: Apache-2.0
- Description: "Meson is a project to create the best possible next-generation build system"
- Homepage: https://mesonbuild.com
- VCS: git+https://github.com/mesonbuild/meson.git

Meson can use multiple backends. The active backend is recorded in `intro-buildoptions.json` under the `backend` option (value: `"ninja"`, `"vs2022"`, `"xcode"`, etc.). The build directory does not contain the backend's actual installed version — only minimum-required version hints (e.g., `ninja_required_version` in `build.ninja`). Running `--version` on a system binary may differ from the version that built the project.

Known backend metadata:
- **ninja**: Homepage https://ninja-build.org, VCS git+https://github.com/ninja-build/ninja.git, License Apache-2.0
- **vs* (Visual Studio)**: Homepage https://visualstudio.microsoft.com/
- **xcode**: Homepage https://developer.apple.com/xcode/

## Goals / Non-Goals

**Goals:**
- Read `meson_version.full` from `meson-info/meson-info.json` in the analyzed project's build directory
- Add a second entry in `metadata.tools.components` with the Meson tool identity
- Include upstream metadata: supplier, license, homepage, VCS, description
- Gracefully handle missing/unreadable `meson-info.json`
- Ensure `mesonsbom` remains first entry; Meson appears second
- Add a third entry for the Meson backend tool, detected generically from `intro-buildoptions.json`
- Map backend name to known upstream metadata
- Set backend version to "unknown" as fallback (no authoritative source in build directory)

**Non-Goals:**
- Detecting Meson version from any other source (CLI, PATH, mesonsbom's own build)
- Tool binary hashes or integrity verification
- Supporting multiple different Meson versions in one SBOM
- Precise backend version detection (not available from build directory alone)

## Design

### Where to read meson-info.json

In `main.cpp`, after `buildDir` is validated but before `SBOMBuilder` construction (or immediately after, before `addTool` for mesonsbom). Same filesystem pattern as other introspection files:

```
fs::path mesonInfoPath = fs::path(buildDir) / "meson-info" / "meson-info.json";
```

### Where to read intro-buildoptions.json

Same directory, for detecting the backend:

```
fs::path optionsPath = fs::path(buildDir) / "meson-info" / "intro-buildoptions.json";
```

### Data flow

```
main.cpp flow:
  1. Parse args (--build-dir, etc.)
  2. Validate build-dir exists
  3. Read meson-info/meson-info.json ──┐
     Extract meson_version.full         │
  4. Read intro-projectinfo.json        │
  5. Read intro-buildoptions.json       │
     Extract backend value              │
  6. Create SBOMBuilder                 │
  7. sbom.addTool(mesonsbom)            │
  8. sbom.addTool(meson)   ◄────────────┘
  9. sbom.addTool(backend) ◄────────────┘
 10. Process dependencies
 11. sbom.writeTo(output)
```

### ToolInfo for Meson

```cpp
sbom.addTool({
    .name = "meson",
    .version = mesonVersion,   // from meson-info.json
    .description = "Meson is a project to create the best possible "
                   "next-generation build system",
    .supplierName = "The Meson Development Team",
    .homepageUrl = "https://mesonbuild.com",
    .vcsUrl = "git+https://github.com/mesonbuild/meson.git",
    .licenses = {"Apache-2.0"}
});
```

### ToolInfo for Backend Build Tool

Read `intro-buildoptions.json`, find the `backend` option, map its value to known metadata:

```cpp
// Read intro-buildoptions.json to detect backend
fs::path optionsPath = fs::path(buildDir) / "meson-info" / "intro-buildoptions.json";
std::string backendName = "unknown";

std::ifstream optIn(optionsPath);
if (optIn) {
    nlohmann::json options;
    optIn >> options;
    for (const auto& opt : options) {
        if (opt.value("name", "") == "backend") {
            backendName = opt.value("value", "unknown");
            break;
        }
    }
}

if (backendName != "unknown") {
    ToolInfo backendInfo;
    backendInfo.name = backendName;
    backendInfo.version = "unknown";  // no authoritative source in build dir

    if (backendName == "ninja") {
        backendInfo.description = "Ninja is a small build system with a focus on speed";
        backendInfo.supplierName = "The Ninja Project";
        backendInfo.homepageUrl = "https://ninja-build.org";
        backendInfo.vcsUrl = "git+https://github.com/ninja-build/ninja.git";
        backendInfo.licenses = {"Apache-2.0"};
    } else if (backendName.rfind("vs", 0) == 0) {
        backendInfo.description = "Microsoft Visual Studio build tools";
        backendInfo.homepageUrl = "https://visualstudio.microsoft.com/";
        backendInfo.supplierName = "Microsoft Corporation";
    } else if (backendName == "xcode") {
        backendInfo.description = "Apple Xcode build system";
        backendInfo.homepageUrl = "https://developer.apple.com/xcode/";
        backendInfo.supplierName = "Apple Inc.";
    }

    sbom.addTool(backendInfo);
}
```

### Error handling

If `meson-info.json` cannot be opened or parsed, emit a warning to stderr and skip adding the Meson tool entry. The SBOM generation continues successfully — Meson tool metadata is an enhancement, not a requirement for a valid SBOM.

```cpp
try {
    std::ifstream in(mesonInfoPath);
    if (in) {
        nlohmann::json j;
        in >> j;
        std::string mesonVersion = j["meson_version"]["full"];
        sbom.addTool({...});
    } else {
        std::cerr << "Warning: Could not open " << mesonInfoPath << std::endl;
    }
} catch (const std::exception& e) {
    std::cerr << "Warning: Could not parse meson-info.json: " << e.what() << std::endl;
}
```

If `intro-buildoptions.json` cannot be opened or `backend` option is missing/unknown, silently skip adding the backend tool entry. No stderr warning — this is a minor enrichment, not a config file. Version SHALL be set to `"unknown"` since the build directory does not contain the backend's actual installed version.

### Output format

```json
{
  "metadata": {
    "tools": {
      "components": [
        {
          "type": "application",
          "name": "mesonsbom",
          "version": "1.0.0",
          "bom-ref": "tool-mesonsbom",
          "purl": "pkg:generic/mesonsbom@1.0.0",
          ...
        },
        {
          "type": "application",
          "name": "meson",
          "version": "1.11.1",
          "bom-ref": "tool-meson",
          "purl": "pkg:generic/meson@1.11.1",
          "description": "Meson is a project to create the best possible next-generation build system",
          "supplier": {
            "name": "The Meson Development Team"
          },
          "licenses": [
            {"license": {"id": "Apache-2.0"}}
          ],
          "externalReferences": [
            {"type": "website", "url": "https://mesonbuild.com"},
            {"type": "vcs", "url": "git+https://github.com/mesonbuild/meson.git"}
          ]
        },
        {
          "type": "application",
          "name": "ninja",
          "version": "unknown",
          "bom-ref": "tool-ninja",
          "purl": "pkg:generic/ninja@unknown",
          "description": "Ninja is a small build system with a focus on speed",
          "supplier": {
            "name": "The Ninja Project"
          },
          "licenses": [
            {"license": {"id": "Apache-2.0"}}
          ],
          "externalReferences": [
            {"type": "website", "url": "https://ninja-build.org"},
            {"type": "vcs", "url": "git+https://github.com/ninja-build/ninja.git"}
          ]
        }
      ]
    }
  }
}
```

## Decisions

### Decision 1: Build-directory meson-info.json is authoritative for Meson version

**Choice:** Read `meson_version` from `meson-info/meson-info.json` in `--build-dir`. This is the only source.

**Rationale:** The proposal explicitly requires the version used to build the analyzed project, not mesonsbom's own Meson version. The build directory's `meson-info.json` is already present (Meson writes it during setup) and contains the exact version.

**Alternatives considered:**
- `meson --version` CLI call — fragile, requires `meson` on PATH, may differ from the version that built the project
- Meson Python API — adds Python dependency, over-engineered
- mesonsbom's own `meson-info.json` — wrong project's version

### Decision 2: Backend version is "unknown"

**Choice:** Set backend tool version to `"unknown"`.

**Rationale:** The build directory does not contain the backend's actual installed version. Ninja's `ninja_required_version` in `build.ninja` is the minimum required version, not the actual installed version. Running `--version` on the system binary may differ from the build-time version. Using "unknown" is honest and avoids misleading SBOM consumers.

**Alternatives considered:**
- `ninja --version` — fragile (not on PATH, wrong version if multiple installations), can differ from build-time version
- `ninja_required_version` from `build.ninja` — this is minimum required, not actual; presenting it as version would be misleading

### Decision 3: Generic backend detection from intro-buildoptions.json

**Choice:** Read the `backend` option value from `intro-buildoptions.json` and map it to known metadata.

**Rationale:** This is the generic approach that works for all backends (ninja, vs*, xcode) without backend-specific logic. Single source of truth in the build directory.

### Decision 4: Supplier is "The Meson Development Team"

**Choice:** Set `supplier.name` to `"The Meson Development Team"`.

**Rationale:** The Meson COPYING file states "Copyright 2016 The Meson development team". This is the canonical author attribution from the project itself.

**Alternatives considered:**
- "Jussi Pakkanen" — personal name, less appropriate for a community project
- Empty — loses provenance information

### Decision 5: Graceful degradation on missing file

**Choice:** Warning to stderr, continue without Meson tool entry.

**Rationale:** `meson-info.json` could theoretically be missing if the user points to an incomplete build directory. The SBOM should still be valid — just less enriched. This is consistent with how other optional metadata (licenses, descriptions) are handled.

### Decision 6: Ordering — addTool call sequence guarantees order

**Choice:** `main.cpp` calls `addTool(mesonsbom)` first, then `addTool(meson)` second, then `addTool(backend)` third. Insertion order = output order.

**Rationale:** Already established pattern from the existing `tool-metadata` capability design.

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| `meson-info.json` format changes between Meson versions (schema drift) | Only read `meson_version.full` — a stable field present in all modern Meson versions. Wrap in try/catch. |
| Analyzed project was built with an ancient Meson that didn't write `meson-info.json` | Graceful skip with stderr warning. SBOM still valid. |
| `bom-ref "tool-meson"` collides with a component named "tool-meson" | Extremely unlikely. If collision occurs, `addComponent` returns false for the component (already added). |
| Wrong Meson version if user manually edits `meson-info.json` | Not our problem — file is authoritative by design. |
| Hardcoded upstream metadata drifts (new license, new homepage) | Acceptable — Meson's license (Apache-2.0) is stable. If homepage/VCS changes, update in a follow-up change. |
| Backend "unknown" version reduces SBOM value | Acceptable trade-off — documenting which backend was used is still useful, even without exact version. |

## Migration Plan

1. Add `meson-info.json` read logic to `main.cpp` after build-dir validation
2. Add `intro-buildoptions.json` read logic for backend detection
3. Add `addTool()` call for Meson with upstream metadata
4. Add `addTool()` call for backend tool with generic mapping
5. Create unit test: verify Meson tool appears as second entry in `tools.components`
6. Create unit test: verify backend tool appears as third entry
7. Create unit test: verify missing `meson-info.json` does not crash
8. Update `tool-metadata` spec to reflect Meson as second tool, backend as third
9. Build, run tests, verify output

Rollback: revert `main.cpp` changes — no data migration needed.

## Open Questions

- Should `meson-info.json` / `intro-*` files version be checked for compatibility? The introspection version (`1.0.0`) appears in the file — but for now, trust it.