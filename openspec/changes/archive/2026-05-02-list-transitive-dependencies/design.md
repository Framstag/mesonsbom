## Context

The `mesonsbom` tool currently extracts dependencies from Meson's `intro-dependencies.json`. However, it only processes the direct dependencies of the main project. In a real-world scenario, these dependencies have their own dependencies (transitive dependencies), which are essential for a complete Software Bill of Materials (SBOM) to satisfy security and compliance requirements.

The current implementation reads the dependencies list once and stops. To support transitive dependencies, the tool needs a way to recursively resolve these dependencies, likely by interacting with the build directory's introspection data for each discovered dependency.

## Goals / Non-Goals

**Goals:**
- Implement a recursive resolution algorithm to find all transitive dependencies.
- Ensure each unique dependency is added as a component in the SBOM exactly once.
- Correctly map the dependency graph (edges) from parents to children, including transitive relations.
- Maintain compatibility with the existing CycloneDX 1.6 JSON output format.

**Non-Goals:**
- Implement a full package manager resolution engine (we rely on what Meson has already resolved in the build directory).
- Resolve dependencies from external network sources (we only use the local `meson-info` data).
- Modify the CLI interface (the `--build-dir` flag remains the primary input).

## Decisions

- **Recursive Traversal**: Use a queue-based (BFS) or stack-based (DFS) approach to traverse the dependency tree. Start with direct dependencies and recursively seek the dependencies of each discovered component until no new dependencies are found.
- **Component Deduplication**: Use a `std::unordered_set` or a map of `bom-ref`s to keep track of already processed components to avoid infinite loops in the case of circular dependencies and to prevent duplicate entries in the SBOM.
- **Data Source**: Continue using the `intro-dependencies.json` and other introspection files. For transitive dependencies, we will look for the corresponding introspection data for each dependency discovered. If no introspection data is available (e.g., for system libraries), the tool SHALL use a `pkg-config` library to programmatically retrieve dependencies.
- **No External Tool Calls**: To ensure security, portability, and performance, the tool MUST NOT invoke external shell commands (e.g., via `popen` or `system`). All dependency information must be retrieved via file reads or library APIs.
- **Edge Representation**: In the `SBOMBuilder`, maintain a mapping of components to their dependencies, ensuring that every resolved transitive dependency is linked to its parent.

## Risks / Trade-offs

- **Risk**: Circular dependencies in the build graph could lead to infinite recursion.
  **Mitigation**: Maintain a "visited" set of `bom-ref`s; if a component is already visited, stop traversing that branch.
- **Risk**: Missing introspection data for some transitive dependencies might lead to an incomplete graph.
  **Mitigation**: Log a warning when a dependency is identified but its own dependency list cannot be resolved from the introspection files.
- **Trade-off**: Recursive resolution increases the time complexity from O(N) to O(V+E) where V is the number of components and E is the number of dependency edges. Given the typical size of build graphs, this is acceptable.
