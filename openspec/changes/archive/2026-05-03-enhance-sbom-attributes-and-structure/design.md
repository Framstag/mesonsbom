## Context

The current SBOM generator (`mesonsbom`) produces a minimal CycloneDX 1.6 JSON output with component names, versions, and basic dependency edges. However, it lacks standard metadata fields that downstream consumers (security scanners, compliance tooling, SBOM aggregators) expect:

- No `purl` (Package URL) identifiers — components cannot be cross-referenced against vulnerability databases.
- No `description` fields — humans and tools cannot understand what a component is.
- No `tools` section — consumers cannot tell which tool generated the SBOM or its version.
- No `license` information — critical for license compliance workflows.
- Redundant `dependsOn` blocks — each transitive dependency creates a separate entry instead of consolidating all dependencies of a component into one array.

The Meson introspection files (`intro-projectinfo.json`, `intro-dependencies.json`) contain some of this data already. For system libraries resolved via `libpkgconf`, the `.pc` files provide descriptions and sometimes license information.

## Goals / Non-Goals

**Goals:**
- Add `purl` (`pkg:generic/<name>@<version>`) to every component in the SBOM.
- Add `description` to each component, sourced from `pkg-config` when available, empty string otherwise.
- Consolidate all dependencies of a single component into one `dependsOn` array per `ref` instead of one entry per edge.
- Add a `metadata.tools` section identifying `mesonsbom` (name and version).
- Extract project license from `intro-projectinfo.json` into `metadata.component.licenses`.
- Include dependency-level license information if available from introspection or `pkg-config`.
- Keep the CycloneDX 1.6 spec version unchanged.

**Non-Goals:**
- Switching to a different SBOM format (e.g., SPDX).
- Adding hashes or evidence for components (out of scope for this change).
- Adding external references (e.g., homepage, VCS links).
- Changing the command-line interface or BFS traversal logic.

## Decisions

### Decision 1: purl format — `pkg:generic`
The Package URL spec defines `pkg:generic/<name>@<version>` for software that has no dedicated purl type. Since most Meson dependencies don't have a one-to-one mapping to a purl type, `generic` is the safest default. Alternatives considered:

- **pkg:meson (custom type)**: Not a registered purl type — would be rejected by purl validators.
- **pkg:github / pkg:sourceforge**: Requires VCS URL data we don't always have.
- **No purl**: Current state, but this is the feature being added.

**Decision**: Use `pkg:generic` with strict encoding per the purl spec (lowercase name, proper percent-encoding for special characters).

### Decision 2: Description source priority
We have two potential sources for component descriptions:
1. Meson introspection data — currently doesn't include descriptions.
2. `libpkgconf` — `.pc` files include a `Description:` field.

**Decision**: Query `libpkgconf` for description when the component is resolved via `pkg-config`. For dependencies resolved purely from Meson introspection without `pkg-config`, leave the description empty. This avoids an extra system call or file read per component.

Implementation: Extend `PkgConfigWrapper` with a `getPackageDescription(pkgName)` method that calls `pkgconf_pkg_find` and reads `pkg->description`.

### Decision 3: Consolidated dependsOn — merge in SBOMBuilder
Currently `addDependency(from, to)` pushes a new JSON object with a single-element `dependsOn` array. Instead, we'll maintain a `std::unordered_map<std::string, std::vector<std::string>>` mapping each `from` ref to a list of `to` refs, then serialize each entry as `{"ref": "...", "dependsOn": [...]}` at write time.

This is purely a structural change in `SBOMBuilder` — the BFS traversal logic in `main.cpp` remains untouched.

### Decision 4: Tools section — populate from CMake/Meson build metadata
Extract the tool's own name and version from the project's build metadata. Since `mesonsbom` builds itself with Meson, we can read `intro-projectinfo.json` of the *current* build (or embed the version via a compile-time define).

**Decision**: Hard-code the tool name and extract version from the `mesonsbom` project's own `projectinfo.json` at the time of SBOM generation. Alternatively, embed a version string in `SBOMBuilder` since `mesonsbom` knows its own version. We'll embed the version in `SBOMBuilder` for simplicity — no additional file reads needed.

### Decision 5: License extraction from Meson introspection
`intro-projectinfo.json` contains a `"license"` array. The main project's license array can be added directly to `metadata.component.licenses` as CycloneDX license objects.

For dependency licenses, if `intro-dependencies.json` includes license information (currently it doesn't), we'll include it. Otherwise, `libpkgconf` can provide license info from `.pc` files via `pkg->license`.

## Risks / Trade-offs

- **[Risk] Full purl compliance**: The `pkg:generic` purl type requires proper percent-encoding of special characters (e.g., `+` in library names). If encoding is incorrect, consumers may reject the purl. **Mitigation**: Use a simple encoding function that replaces spaces and special characters per the purl specification.
- **[Risk] Missing descriptions**: Not all dependencies resolve via `pkg-config`. Descriptions for Meson-only dependencies will be empty, which may reduce SBOM quality. **Mitigation**: Accept empty strings as valid per CycloneDX spec — descriptions are optional in the standard.
- **[Trade-off] Consolidated dependsOn changes the output structure**: Tools consuming the current format will need to adapt to the new structure. However, consolidated dependsOn is the standard CycloneDX format — the previous per-edge format was non-standard.
- **[Risk] License field may be inaccurate**: The `intro-projectinfo.json` license field may contain `"unknown"` (as seen in the test project). **Mitigation**: Only include license information when a meaningful value is available; skip the licenses array entirely if all entries are `"unknown"`.
