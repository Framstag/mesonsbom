## 1. Timestamp Implementation

- [x] 1.1 Add `makeTimestamp()` helper that captures `std::time(nullptr)`, formats via `std::gmtime` + `std::strftime` as `YYYY-MM-DDTHH:mm:ssZ`, and returns `std::string`. Include null-check fallback. (1)
- [x] 1.2 Add unit tests for timestamp format: validates ISO 8601 regex, UTC Z suffix, stable on repeated calls from same instance. (2)

## 2. SBOM Integration

- [x] 2.1 Add `std::string timestamp_` member, generate in `SBOMBuilder` constructor by calling `makeTimestamp()`. (1)
- [x] 2.2 Include `metadata.timestamp` in `writeTo()` output JSON. Verify existing tests still pass. (1)

## 3. End-to-End Tests

- [x] 3.1 Add integration tests in `test_json.cpp`: `metadata.timestamp` present in output, matches ISO 8601 regex, same instance stable, cross-instance variation. (2)