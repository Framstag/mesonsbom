## 1. Build System Configuration

- [x] 1.1 Add `args: ['-r', 'tap']` to the `test('mesonsbom-tests', ...)` call in `meson.build`.
- [x] 1.2 Add `protocol: 'tap'` to the same `test()` call in `meson.build`.
- [x] 1.3 Rebuild the test executable and run `meson test -C build --verbose` to verify individual `TEST_CASE`s are shown.

## 2. Verification

- [x] 2.1 Verify that `meson test -C build` output lists each Catch2 `TEST_CASE` as a separate test result.
- [x] 2.2 Verify that passing tests show as `ok` and failing tests show as `not ok` in the test output.
- [x] 2.3 Verify that the overall test suite still reports correct pass/fail totals.