## Context

The test binary `mesonsbom_tests` contains 8 individual Catch2 `TEST_CASE`s across multiple source files. However, `meson test -C build` currently reports only a single test result (`1/1 mesonsbom_tests`), because Meson's default `exitcode` protocol only tracks whether the test binary as a whole passed or failed. Developers must open the test log file to see individual test case results.

Both Meson and Catch2 have built-in support for TAP (Test Anything Protocol), which provides per-test-case reporting. No new dependencies are required.

## Goals / Non-Goals

**Goals:**
- Make `meson test -C build` show each Catch2 `TEST_CASE` as a separate test result.
- Ensure passing, failing, and skipped test cases are visibly distinguished.
- Use only built-in features — no new dependencies, no external scripts.
- Preserve all existing test logic — test registrations, assertions, and `main.cpp` remain unchanged.

**Non-Goals:**
- Changing the Catch2 version or test framework.
- Modifying test source code or test structure.
- Adding custom Meson test runners or discovery scripts.
- Supporting non-Catch2 test frameworks.

## Decisions

### Decision 1: TAP reporter over GTest protocol
Meson offers `protocol: 'gtest'` for Google Test, but Catch2's CLI flags differ from GTest's. The TAP protocol is framework-agnostic and both Meson and Catch2 support it natively:

- **Catch2** has a built-in TAP reporter (`-r tap`) that outputs one `ok`/`not ok` line per `TEST_CASE`.
- **Meson**'s `protocol: 'tap'` parses these lines and reports each as an individual test.

### Decision 2: No wrapper scripts needed
Unlike some projects that use wrapper scripts to parse `--list-tests` output and register individual `test()` calls in `meson.build`, the TAP approach requires no scripts — it's a configuration-only change.

### Decision 3: Reporter flag passed via `test()` `args`
The `-r tap` argument is passed to the test binary through Meson's `test()` `args:` parameter. This avoids modifying test source code or `main()`.

## Risks / Trade-offs

- **[Risk] TAP output on test failure**: In TAP mode, Catch2 still reports individual test case results in `#` comments on failure, plus the binary exits with a non-zero code. **Mitigation**: Meson's TAP handler correctly interprets non-zero exit as overall failure while still showing individual results.
- **[Risk] Test log format change**: Any external tooling parsing the old console reporter output from test logs would see different format. **Mitigation**: The TAP format is well-structured and more parseable than the default console output.
- **[Risk] Catch2 v2 compatibility**: The TAP reporter was added in Catch2 v3.x. **Mitigation**: The project already uses Catch2 v3.14.0, verified by `pkg-config`.
- **[Risk] Test output verbosity**: TAP reporter outputs test case names but not individual assertion details by default (those go in `#` comments on failure). **Mitigation**: This is acceptable — `meson test --verbose` or the test log file show full details on failures.
