## Context

The current implementation of transitive dependency resolution in `mesonsbom` uses a placeholder logic when Meson introspection files are missing for a component. While `libpkgconf` was added to the build system, it is not yet utilized in the code. System libraries (external to the Meson subproject system) currently have their transitive dependencies ignored or not resolved, leading to incomplete SBOMs.

## Goals / Non-Goals

**Goals:**
- Fully implement programmatic querying of package dependencies using `libpkgconf`.
- Decouple the `libpkgconf` C-API from the main application logic via a wrapper class.
- Maintain the existing BFS traversal logic for transitive resolution.
- Ensure zero memory leaks by managing the `pkgconf_ctx_t` lifecycle correctly.

**Non-Goals:**
- Modifying the Meson introspection logic.
- Changing the CLI interface of the tool.
- Supporting non-pkg-config based system dependencies.

## Decisions

### 1. Wrapper Class for libpkgconf
**Decision**: Create a `PkgConfigWrapper` class.
**Rationale**: The `libpkgconf` API is a C-style API with manual memory management (`pkgconf_ctx_new`, `pkgconf_ctx_free`). Wrapping this in a C++ class using RAII ensures that resources are always cleaned up, even in the case of exceptions. It also provides a cleaner interface (e.g., returning `std::vector<std::string>`) to the rest of the application.
**Alternatives**: 
- Direct calls in `main.cpp`: Leads to scattered cleanup code and higher risk of leaks.
- Functional wrapper: A simple set of functions; less cohesive than a class.

### 2. Dependency Query Strategy
**Decision**: Use `pkgconf_set_pkg_name` followed by querying requirements.
**Rationale**: This is the standard way to identify a package and retrieve its requirements. It maps directly to the functionality of `pkg-config --print-requires`.
**Alternatives**:
- Manual parsing of `.pc` files: Too complex and redundant since `libpkgconf` already does this.

## Decisions

### 1. Wrapper Class for libpkgconf
**Decision**: Create a `PkgConfigWrapper` class.
**Rationale**: The `libpkgconf` API is a C-style API with manual memory management (`pkgconf_ctx_new`, `pkgconf_ctx_free`). Wrapping this in a C++ class using RAII ensures that resources are always cleaned up, even in the case of exceptions. It also provides a cleaner interface (e.g., returning `std::vector<std::string>`) to the rest of the application.
**Alternatives**: 
- Direct calls in `main.cpp`: Leads to scattered cleanup code and higher risk of leaks.
- Functional wrapper: A simple set of functions; less cohesive than a class.

### 2. Dependency Query Strategy
**Decision**: Use `pkgconf_set_pkg_name` followed by querying requirements.
**Rationale**: This is the standard way to identify a package and retrieve its requirements. It maps directly to the functionality of `pkg-config --print-requires`.
**Alternatives**:
- Manual parsing of `.pc` files: Too complex and redundant since `libpkgconf` already does this.

### 3. Integration Point
**Decision**: Update `loadDependencies` helper in `src/main.cpp`.
**Rationale**: `loadDependencies` is already the centralized place for fetching dependencies. Adding the `PkgConfigWrapper` call here maintains the existing architectural flow: first check Meson subprojects, then fallback to system libraries.

### 4. Handling Unresolvable Dependencies
**Decision**: Issue a warning to `stderr` and continue.
**Rationale**: Not every dependency will be a `pkg-config` package. Hard-failing would prevent the generation of the rest of the SBOM. A warning informs the user of the gap without halting the process.

## Risks / Trade-offs

- **[Risk]** `libpkgconf` version mismatches: Different system versions of `pkgconf` might have slight API variations.
- **[Mitigation]** Use the most stable and widely available API calls; include the dependency strictly in `meson.build`.
- **[Risk]** Performance overhead of context creation: Creating a new context for every component might be slow.
- **[Mitigation]** Consider reusing the context if the number of system libraries is high, though for most SBOMs, the overhead is negligible.
