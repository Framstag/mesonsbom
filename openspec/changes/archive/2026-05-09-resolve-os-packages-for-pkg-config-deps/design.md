## Context

The tool currently resolves system library dependencies via libpkgconf (pkg-config library API). Each dependency is identified only by its pkg-config name and version (e.g., "libssl 3.0.14") and added directly to the SBOM with a `pkg:generic/<name>@<version>` purl.

On OS-managed systems, these `.pc` files are owned by OS packages. The file path of each `.pc` file (`pkgconf_pkg_t.filename`) is the key to mapping: querying the system package manager for the owning package yields the OS package identity.

The SBOM currently creates components inline during BFS traversal of the dependency graph. This one-pass approach makes deduplication impossible when two pkg-config deps map to the same OS package.

## Goals / Non-Goals

**Goals:**
- Map each pkg-config dependency to an OS package where possible
- OS package identity takes precedence for component name, version, purl, and supplier
- Preserve original pkg-config data in `evidence.identity[]` and `properties`
- Deduplicate multiple pkg-config deps mapping to the same OS package
- Graceful fallback to pkg-config identity when OS resolution fails
- Abstract OS detection for future Windows/Apple support

**Non-Goals:**
- Not querying remote package repositories — only local package manager database
- Not resolving OS packages for Meson subproject dependencies
- Not supporting NixOS, Guix, or other non-FHS systems (falls back to pkg-config)
- Not adding new CLI flags or changing existing CLI interface

## Decisions

### Decision 1: Two-phase pipeline instead of inline BFS

Current code walks deps and adds components immediately during BFS. This cannot deduplicate by OS package because the OS package identity isn't known until after all deps are collected.

```
Current:  collect dep → resolve pkg-config → add component → BFS next
New:
  Phase 1 (Collect):  Walk entire dep graph, produce raw list of
                       {pkgConfigName, version, pcFilePath, dependencies}
  Phase 2 (Resolve  ): For each item, query OS package manager.
                       Result: {osPkgName, osVersion, packageManager, distribution}
  Phase 3 (Dedupe  ): Group resolved items by osPkgName.
                       Identical OS packages → one component.
                       Failed resolutions → kept as individual pkg-config components.
  Phase 4 (Build   ): Feed deduplicated list to SBOMBuilder.
```

**Alternatives considered:**
- **Deferred component creation with ID mapping**: Simpler but makes error handling fragile
- **Post-hoc merge on SBOM JSON**: Works but breaks the clean CycloneDX model

**Rationale:** Clear separation of concerns. Each phase is independently testable. Easy to add future resolvers (e.g., Windows registry, Homebrew) without touching main loop.

### Decision 2: Abstract `OsPackageResolver` with PM detection at runtime

```cpp
class OsPackageResolver {
public:
    // Detect available package manager
    PackageManager detectPackageManager();
    
    // Query OS package owning a file
    // Returns empty optional if not found
    std::optional<OsPackage> resolve(const std::string& pcFilePath);
    
private:
    PackageManager pm_;
    std::unordered_map<std::string, OsPackage> cache_; // path → package
};
```

Detection logic: check `which pacman`, `which dpkg-query`, `which rpm`, `which apk` in order. First found wins.

**Alternatives considered:**
- **Compile-time #ifdef per OS**: Brittle, can't handle cross-distro binaries
- **Single hardcoded command per distro via /etc/os-release**: More fragile than binary detection

**Rationale:** Runtime detection using binary existence is robust. Same binary works on Arch, Debian, Fedora, Alpine. The cache prevents redundant subprocess calls when multiple `.pc` files belong to the same OS package.

### Decision 3: purl uses generic type with OS qualifier

Format: `pkg:generic/<osPkgName>@<osVersion>?os=<pm>`

Examples:
- `pkg:generic/openssl@3.0.14-1?os=pacman`
- `pkg:generic/libcurl@8.20.0-5?os=pacman`

**Rationale:** CycloneDX purl types only support well-known package types (pypi, npm, etc.). "pacman" and "dpkg" are not registered purl types. The generic type with the `os` qualifier is the spec-compliant escape hatch. Consumers can parse the qualifier for OS-specific lookups.

### Decision 7: Meson bridge deps — intro-dependencies.json `dependencies` field

Meson introspection JSON (`intro-dependencies.json`) entries include a `dependencies` array listing Meson-discovered transitive children. This is separate from pkg-config's `Requires`. Some packages (e.g., Qt5 modules via `dependency('qt5', modules: [...])`) list children in this field but their `.pc` file has zero `Requires`. The pkg-config-only fallback fails to discover these children.

**Solution:** Before falling back to pkg-config in `loadDependencies()` for a non-main-project component, check if the main `intro-dependencies.json` has a matching entry with a `dependencies` field. If so, return those children as discovered deps with `type: "pkgconfig"`. Self-references (component appearing as its own dependency) are filtered out.

**Resolution order per component:**

```
1. Subproject introspection:  buildDir/meson-info/subprojects/<name>/intro-dependencies.json
2. Meson bridge:              intro-dependencies.json entry's "dependencies" field
3. pkg-config fallback:       pkgconf_getDependencies(<name>) -> .pc Requires
4. Not found:                 warning and continue
```

### Decision 4: SBOM evidence and properties structure

When OS package is resolved, component output:

```json
{
  "type": "library",
  "name": "curl",
  "version": "8.20.0-5",
  "purl": "pkg:generic/curl@8.20.0-5?os=pacman",
  "supplier": { "name": "Arch Linux" },
  "evidence": {
    "identity": [
      {
        "field": "purl",
        "confidence": 1.0,
        "concludedValue": "pkg:generic/curl@8.20.0-5?os=pacman",
        "methods": [{
          "technique": "manifest-analysis",
          "confidence": 1.0,
          "value": "os:package=pacman|os:name=curl|os:version=8.20.0-5"
        }],
        "tools": ["tool-mesonsbom"]
      },
      {
        "field": "name",
        "confidence": 1.0,
        "concludedValue": "libcurl",
        "methods": [{
          "technique": "manifest-analysis",
          "confidence": 1.0,
          "value": "pkg-config:libcurl|pkg-config-version=7.88.1"
        }],
        "tools": ["tool-mesonsbom"]
      }
    ],
    "occurrences": [{
      "location": "/usr/lib/pkgconfig/libcurl.pc"
    }]
  },
  "properties": [
    { "name": "discovery:method",   "value": "pkg-config" },
    { "name": "pkg-config:name",    "value": "libcurl" },
    { "name": "pkg-config:version", "value": "7.88.1" },
    { "name": "os:distribution",    "value": "arch" },
    { "name": "os:package",         "value": "curl" },
    { "name": "os:package-version", "value": "8.20.0-5" }
  ]
}
```

When OS resolution fails, output is identical to current behavior (no `evidence`, no `properties`).

### Decision 5: OS release detection via `/etc/os-release`

Use `std::ifstream` to read `/etc/os-release`, parse `ID=arch` and `NAME=Arch Linux`. The `NAME` field becomes the supplier name. The `ID` becomes the `os:distribution` property value.

```cpp
struct OsRelease {
    std::string id;      // "arch", "debian", "ubuntu", "fedora"
    std::string name;    // "Arch Linux", "Debian GNU/Linux"
    std::string versionId; // optional, e.g., "11" for Debian
};
```

### Decision 6: Confidence always 1.0

Per user decision. The evidence chain is deterministic — we either found the OS package or we didn't. No probabilistic inference.

## Sequence Diagram

```
main.cpp                          PkgConfigWrapper    OsPackageResolver    SBOMBuilder
    │                                    │                   │                 │
    │── Phase 1: Collect ────────────────│                   │                 │
    │   │                                │                   │                 │
    │   │── loadDependencies(BFS) ──────→│                   │                 │
    │   │                                │── .filename ─────→│                 │
    │   │                                │   .id .version    │                 │
    │   │←──────────────────────────────│                   │                 │
    │   │                               │                    │                 │
    │── Phase 2: Resolve OS packages ───│                    │                 │
    │   │                               │                    │                 │
    │   │──── resolve(pcFilePath) ──────────────────────────→│                 │
    │   │                               │                    │── popen()       │
    │   │                               │                    │── parse output  │
    │   │←──────────────────────────────────────────────────│                 │
    │   │    {osPkg, osVer, pm, distro} │                    │                 │
    │   │                               │                    │                 │
    │── Phase 3: Deduplicate ───────────│                    │                 │
    │   │ group by osPkgName            │                    │                 │
    │   │ failed → keep as pkg-config   │                    │                 │
    │   │                               │                    │                 │
    │── Phase 4: Build SBOM ────────────│                    │                 │
    │   │── sbom.addComponent(...) ──────────────────────────────────────────→│
    │   │── sbom.addEvidence(...) ───────────────────────────────────────────→│
    │   │── sbom.addProperties(...) ─────────────────────────────────────────→│
    │   │── sbom.addDependency(...) ─────────────────────────────────────────→│
    │   │                               │                    │                 │
    │── sbom.writeTo() ──────────────────────────────────────────────────────→│
```

## Data Structures

```cpp
// Collected raw dependency before OS resolution
struct RawDependency {
    std::string pkgConfigName;
    std::string pkgConfigVersion;
    std::string pcFilePath;       // from pkgconf_pkg_t.filename
    std::string description;       // from pkg-config Description:
    std::vector<std::string> pkgConfigDeps; // transitive children to BFS
    bool isPkgConfig = false;      // true if resolved via pkg-config
};

// Result of OS package resolution
struct OsPackage {
    std::string name;              // e.g., "curl"
    std::string version;           // e.g., "8.20.0-5"
    std::string packageManager;    // e.g., "pacman"
};

// Resolved dependency (after Phase 2-3)
struct ResolvedDependency {
    std::string osName;            // OS package name (empty if failed)
    std::string osVersion;         // OS package version (empty if failed)
    std::string osPackageManager;  // PM identifier (empty if failed)
    
    std::string pkgConfigName;     // always present
    std::string pkgConfigVersion;  // always present  
    std::string pcFilePath;        // always present
    std::string description;       // always present
    
    bool hasOsPackage() const { return !osName.empty(); }
};
```

## Risks / Trade-offs

| Risk | Mitigation |
|---|---|
| **Subprocess per dep is slow** (10 deps × fork) | Cache per .pc path in `OsPackageResolver`. Docker builds typically have ~10-20 pkg-config deps, subprocess cost is negligible. |
| **Package manager DB missing** (minimal container) | `resolve()` returns empty optional → clean fallback to pkg-config |
| **`popen()` failure or timeout** on broken PM | Wrap in try/catch + timeout via `alarm()` or non-blocking pipe. Fallback on any exception. |
| **Cross-compilation sysroot** (.pc files not on host) | `pcFilePath` points into sysroot → PM query fails → fallback. No special handling needed. |
| **Windows/Apple detection** | Abstract `OsPackageResolver::detectPackageManager()` is the only place that needs platform logic. Future implementations replace the Linux `popen()` calls with Windows Registry or `brew` query. |
| **Two-phase BFS doubles traversal** | First phase stores `RawDependency` list (memory only). Second phase reuses it without re-scanning filesystem. No IO duplication, only memory. |
