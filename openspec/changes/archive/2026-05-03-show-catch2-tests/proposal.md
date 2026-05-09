## Why

Currently, `meson test -C build` reports the Catch2 test binary as a single test (`1/1 mesonsbom_tests OK`), even though the binary contains multiple individual `TEST_CASE`s. This makes it impossible to see which test cases pass, fail, or skip at a glance. Developers must inspect the test log manually to see per-test-case results.

## What Changes

- **Enable Catch2's TAP reporter**: Pass `-r tap` to the test binary so it outputs TAP (Test Anything Protocol) format, where each `TEST_CASE` emits its own result line.
- **Set Meson test protocol to `tap`**: Change the `test()` call in `meson.build` to use `protocol: 'tap'`, so Meson parses the TAP output and reports each test case individually.
- No new dependencies are required — both Catch2's TAP reporter and Meson's TAP protocol are built-in features.

## Capabilities

### New Capabilities
- `catch2-test-protocol`: Activate Catch2's TAP reporter for Meson test integration so each individual `TEST_CASE` is reported as a separate test result in `meson test` output.

### Modified Capabilities
*(none)*

## Impact

- **meson.build**: Add `args: ['-r', 'tap']` and `protocol: 'tap'` to the `test('mesonsbom-tests', ...)` call.
- **tests/main.cpp**: No changes needed — Catch2's TAP reporter is built-in and selected via the `-r` flag.
- **tests/**: Integration tests may need adjustment if they rely on the test binary's default console reporter output format.