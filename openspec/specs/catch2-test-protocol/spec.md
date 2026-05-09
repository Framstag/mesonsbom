# Catch2 Test Protocol Integration

## Purpose

Configure Catch2's built-in TAP reporter and Meson's TAP protocol so that each individual Catch2 `TEST_CASE` is reported as a separate test result when running `meson test`.

## Requirements

### Requirement: Individual Test Case Reporting via TAP Protocol
The system SHALL configure Catch2's built-in TAP reporter and Meson's TAP protocol so that each individual Catch2 `TEST_CASE` is reported as a separate test result when running `meson test`.

#### Scenario: Individual test cases visible in meson test output
- **WHEN** the user runs `meson test -C build`
- **THEN** each Catch2 `TEST_CASE` SHALL appear as a separate test result line in the output.
- **AND** the test count SHALL reflect the number of `TEST_CASE`s, not just the number of test binaries.
- **AND** passing, failing, and skipped test cases SHALL be distinguished in the output.

#### Scenario: TAP reporter enabled via command-line argument
- **WHEN** the test binary is executed
- **THEN** it SHALL be invoked with the `-r tap` argument to enable Catch2's TAP output format.
- **AND** the TAP reporter SHALL be a built-in Catch2 reporter requiring no additional code changes.

#### Scenario: Meson protocol set to tap
- **WHEN** the test is defined in `meson.build`
- **THEN** the `test()` call SHALL specify `protocol: 'tap'`.
- **AND** Meson SHALL parse the TAP output to extract individual test case results.
