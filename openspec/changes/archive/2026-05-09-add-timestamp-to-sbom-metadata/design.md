## Context

Current SBOMBuilder generates CycloneDX 1.6 JSON output but omits the optional `metadata.timestamp` field. The CycloneDX 1.6 specification recommends (SHOULD level) a timestamp in ISO 8601 format indicating when the BOM was generated. This field enables downstream consumers to evaluate freshness, establish audit timelines, and support build reproducibility analysis.

## Goals / Non-Goals

**Goals:**
- Every generated SBOM includes `metadata.timestamp` in ISO 8601 UTC format.
- Timestamp is captured automatically — no user input or CLI flag.
- No new external dependencies.
- Timestamp is stable across repeated `writeTo` calls on the same instance.

**Non-Goals:**
- Sub-second precision. Seconds granularity is sufficient for BOM generation timestamps.
- Timezone support beyond UTC. CycloneDX recommends UTC (Z suffix).
- User-configurable timestamps. The timestamp reflects actual generation time, not a user-chosen value.
- Thread-safe time capture. All SBOM operations are single-threaded.

## Decisions

### 1. Time source: `std::time` over `std::chrono::system_clock::now`

**Decision**: Use `std::time(nullptr)` to capture current time at construction.

**Alternatives considered**:
- **`std::chrono::system_clock::now()`**: Provides sub-second precision but requires more verbose formatting. Not worth the complexity for seconds-level precision.
- **`std::chrono::zoned_time` / C++20 timezone**: Requires `<chrono>` with C++20 timezone support which is not universally available across compilers.
- **External library (date, fmt)**: Would add dependencies.

**Rationale**: `std::time_t` with `std::gmtime` and `std::strftime` is the simplest portable path to ISO 8601 UTC timestamps. It's C++98-era standard, works everywhere, and seconds granularity is sufficient for SBOM timestamps.

### 2. Capture at construction, stable across serialization

**Decision**: Capture timestamp once in `SBOMBuilder` constructor and store as a `std::string`. Use the same timestamp for all `writeTo` calls on that instance.

**Rationale**:
- Follows the same pattern as `serialNumber_` — consistency across fields.
- Ensures `writeTo` is idempotent (same output each call).
- Constructor is when the SBOM generation starts — closest proxy for "when was this BOM created."

### 3. ISO 8601 format: `YYYY-MM-DDTHH:mm:ssZ`

**Decision**: Format as `YYYY-MM-DDTHH:mm:ssZ` using `std::strftime` with `"%Y-%m-%dT%H:%M:%SZ"`.

**Rationale**:
- Matches CycloneDX 1.6 spec recommendation for UTC timestamp format.
- `Z` suffix indicates UTC per ISO 8601 and RFC 3339.
- All lowercase `T` and `Z` per convention.

## Risks / Trade-offs

- **[Risk] std::gmtime returns nullptr on error**: If `time_t` is negative or out of range, `gmtime` returns `nullptr`. **Mitigation**: Check return value and fall back to a minimal string (`"1970-01-01T00:00:00Z"`). Practically impossible on modern hardware.
- **[Risk] Thread safety**: `std::gmtime` returns a pointer to internal static storage (on some implementations, `gmtime_r` is the thread-safe variant). **Mitigation**: The timestamp is captured once in the constructor — no concurrent access to `gmtime`. Acceptable.
- **[Trade-off] Seconds precision only**: Sub-second precision is lost. **Mitigation**: BOM timestamp at second granularity is standard practice. Most SBOM consumers don't distinguish sub-second differences.
