## Context

Current SBOMBuilder generates CycloneDX 1.6 JSON output but omits the top-level `serialNumber` field. CycloneDX 1.6 mandates `serialNumber` as a URN in the format `urn:uuid:<UUIDv4>` — without it, generated SBOMs are technically invalid and may be rejected by downstream tooling (scanners, aggregators, compliance systems).

## Goals / Non-Goals

**Goals:**
- Every generated SBOM includes a valid `serialNumber` field.
- UUID is generated automatically — no user input or CLI flag.
- No new external dependencies.
- Each SBOMBuilder instance produces a different serial number.

**Non-Goals:**
- Cryptographic security or audit-grade uniqueness. Collision probability of UUIDv4 (~2⁻¹²² per instance) is acceptable for SBOM identification.
- User-configurable serial numbers. The spec treats serialNumber as an auto-generated document identifier, not a user-chosen field.
- UUID parsing, validation, or conversion utilities beyond generation.

## Decisions

### 1. UUIDv4 generator: built-in over external library

**Decision**: Implement a small UUIDv4 generation function using C++20 standard facilities (`std::random_device`, `std::mt19937`). Seed the PRNG once at startup with entropy from `std::random_device` XOR-mixed with `std::time(nullptr)`.

**Alternatives considered**:
- **libuuid**: Common on Linux but adds a build dependency and requires `-luuid` linkage. Not worth it for ~30 lines of code.
- **Boost.UUID**: Heavy dependency pull for a single feature.
- **system(3) / /proc/sys/kernel/random/uuid**: Fragile, platform-specific, unportable.

**Rationale**: UUIDv4 is mechanically simple — 122 bits of randomness formatted as hex with fixed version (4) and variant (10) nibbles. A standalone function is ~15 lines, trivially verifiable, and eliminates all dependency concerns.

**Seeding strategy**: On some platforms (MinGW, older QEMU, containers with low entropy), `std::random_device` can be deterministic — returning the same sequence every run. To guarantee different seeds across application starts, we use a static `std::mt19937` seeded once at first use with `std::random_device{}() ^ std::time(nullptr)`. The XOR with current time ensures variation even when `random_device` is broken. This avoids per-`SBOMBuilder` PRNG construction overhead and guarantees process-unique RNG state.

### 2. Generate at construction, include at serialization

**Decision**: Generate the UUID in the `SBOMBuilder` constructor and store it as a member string. Include `serialNumber` in the output JSON inside `writeTo()`.

**Rationale**:
- One UUID per BOM instance — guarantees consistency even if `writeTo` is called multiple times.
- No risk of different serial numbers across serialization calls.
- Constructor is the natural place since `serialNumber` is a fixed BOM identity attribute.

### 3. UUID format: lowercase hex with dashes

**Decision**: Generate UUID in standard `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx` format, all lowercase. Prepend `urn:uuid:` at serialization time.

**Rationale**: RFC 4122 specifies lowercase hex. Uppercase works too but lowercase is convention. Pre-pending the URN scheme at serialization keeps the stored value reusable (e.g., future `bom-ref` use cases).

## Risks / Trade-offs

- **[Risk] std::random_device determinism**: On some platforms (e.g., MinGW, older QEMU, containers), `std::random_device` may be deterministic or return the same sequence each run. **Mitigation**: XOR seed with `std::time(nullptr)` so every application start produces a different PRNG state, regardless of `random_device` behavior.
- **[Risk] Thread safety**: `std::mt19937` is not thread-safe. **Mitigation**: The PRNG is a function-local `static` — initialized once on first call. If `SBOMBuilder` is ever used from multiple threads, a mutex or thread-local PRNG would be needed. For current single-threaded usage, safe.
- **[Trade-off] Time granularity**: Seeding with `time(nullptr)` (1-second resolution) means two starts within the same second on a broken `random_device` platform could collide. **Mitigation**: Acceptable for non-cryptographic UUIDv4. BOM `version` field provides additional disambiguation.