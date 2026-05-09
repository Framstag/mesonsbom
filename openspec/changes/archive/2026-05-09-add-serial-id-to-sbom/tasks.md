## 1. UUID Generation

- [x] 1.1 Implement `generateUuidV4()` free function using a function-local `static std::mt19937` seeded once with `std::random_device{}() ^ std::time(nullptr)` (C++20). Outputs standard `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx` format. (3)

- [x] 1.2 Add unit tests for UUID generation: validates format regex, variant/version nibbles correct, multiple calls produce different values. Ensure consecutive test starts yield different first UUID (by injecting `std::time` offset via test helper). (3)

## 2. SBOM Integration

- [x] 2.1 Add `std::string serialNumber_` member to `SBOMBuilder`. Generate once in constructor by calling `generateUuidV4()`. (1)

- [x] 2.2 Include `serialNumber` field in `writeTo()` output JSON as `urn:uuid:<uuid>`. Verify existing tests still pass. (1)

- [x] 2.3 Add integration-style unit tests: generated SBOM JSON contains `serialNumber`, value matches `urn:uuid:<UUIDv4>` format, same instance yields same serial on repeated `writeTo` calls. (3)