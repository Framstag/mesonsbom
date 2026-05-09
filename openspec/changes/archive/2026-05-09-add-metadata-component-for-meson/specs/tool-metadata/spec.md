## MODIFIED Requirements

### Requirement: mesonsbom appears first in tools.components array
mesonsbom SHALL always be the first entry in the `metadata.tools.components` array. Meson SHALL appear as the second entry when `meson-info.json` is present and readable. The Meson backend build tool SHALL appear as the third entry when `intro-buildoptions.json` is present and readable. Any future tools SHALL appear after the backend entry.

#### Scenario: mesonsbom is first tool in the array
- **WHEN** an SBOM is generated
- **THEN** the first entry in `metadata.tools.components` SHALL have `name` equal to `"mesonsbom"`
- **AND** subsequent entries (if any) SHALL appear after mesonsbom

#### Scenario: Meson is second tool in the array
- **WHEN** an SBOM is generated with a valid build directory
- **AND** `meson-info/meson-info.json` is present and readable
- **THEN** the second entry in `metadata.tools.components` SHALL have `name` equal to `"meson"`

#### Scenario: Backend tool is third in the array
- **WHEN** an SBOM is generated with a valid build directory
- **AND** `meson-info/intro-buildoptions.json` is present and readable
- **AND** the `backend` option has a known value
- **THEN** the third entry in `metadata.tools.components` SHALL have `name` matching the backend value
