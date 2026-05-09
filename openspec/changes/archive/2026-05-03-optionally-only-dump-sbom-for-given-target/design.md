## Context

The current SBOM generator reads `intro-projectinfo.json` as the single source for the main component name and version, and `intro-dependencies.json` to discover all direct dependencies. For single-target projects this is fine, but multi-target projects (e.g., a library + test executable + CLI tool) get a unified SBOM that doesn't differentiate between targets. Meson's `intro-targets.json` already contains per-target metadata — including each target's `dependencies` array — but this data is currently unused.

## Goals / Non-Goals

**Goals:**
- Add a `-t` / `--target` CLI option that accepts a target name from `intro-targets.json`.
- When `--target` is specified: read `intro-targets.json`, resolve the target, use its name as the main SBOM component, and scope dependencies to only those listed in that target's `dependencies` field.
- When `--target` is omitted: behavior is identical to current full-project generation.
- No breaking changes to the existing CLI interface.

**Non-Goals:**
- Modifying `intro-targets.json` or any Meson introspection files.
- Adding target versioning — the project version from `intro-projectinfo.json` is used for all targets.
- Filtering transitive dependencies per-target (only direct dependency scope is target-specific; transitive resolution proceeds normally for included deps).

## Decisions

### Decision 1: Target resolution by `name`, not `id`
`intro-targets.json` entries have both `name` (e.g., `"mesonsbom"`) and `id` (e.g., `"mesonsbom@exe"`). Using the human-readable `name` for the `--target` argument is more intuitive for users. The lookup iterates the array and matches `entry["name"] == targetArg`.

### Decision 2: Project version still from `intro-projectinfo.json`
Targets in `intro-targets.json` do not carry their own version strings. The project-level version from `intro-projectinfo.json` is used for the main component even when `--target` is specified. Only the component *name* changes to the target's name.

### Decision 3: Dependency scope via name-matching
The target's `dependencies` field is an array of strings (e.g., `["nlohmann_json", "cxxopts"]`). The direct dependency list from `intro-dependencies.json` is filtered to only include entries whose `name` matches this array. Transitive BFS resolution then proceeds normally from those filtered direct dependencies.

### Decision 4: `--target` parsed after `--version`/`--help` guards
The `--target` option is added to the existing `cxxopts` block and only checked if the tool proceeds past the `--version` and `--help`/`--build-dir` guards.

## Risks / Trade-offs

- **[Risk] Target not found**: User specifies a target name that doesn't match any entry in `intro-targets.json`. **Mitigation**: Print an error message listing available targets and exit with code 1.
- **[Risk] Target with no dependencies**: A target might have an empty `dependencies` array. **Mitigation**: Generate a minimal SBOM with just the target component and no dependency edges — valid CycloneDX.
- **[Risk] intro-targets.json parse failure**: The file might be malformed or missing. **Mitigation**: Return a clear error and exit with code 1, same as other missing introspection files.
- **[Risk] Version ambiguity**: Different targets in a project may conceptually have different versions. **Mitigation**: Documented non-goal — the project-level version is used for all targets. Users with distinct version-per-target needs can use `--build-dir` per-target with separate builds.
