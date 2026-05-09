project('mesonsbom', 'cpp', version : '1.1.0', license : 'GPL-3.0-or-later', default_options : ['cpp_std=c++20'])

project_author = 'Tim Teulings'

# Dependencies
json_dep = dependency('nlohmann_json', required : false)
cxxopts_dep = dependency('cxxopts', required : false)
catch2_dep = dependency('catch2', required : false)
pkgconf_dep = dependency('pkgconf', required : true)

# Source files
src = files('src/main.cpp', 'src/pkg_config_wrapper.cpp')

# We add the explicit include directory for pkgconf as a fallback because
# some distributions place the headers in /usr/include/pkgconf/
pkgconf_inc = include_directories('/usr/include/pkgconf')

mesonsbom_version = '-DMESONSBOM_VERSION="' + meson.project_version() + '"'

executable('mesonsbom', src,
  dependencies : [json_dep, cxxopts_dep, pkgconf_dep],
  include_directories: pkgconf_inc,
  cpp_args: mesonsbom_version,
  link_args: ['-lpkgconf'])

# Unit and integration tests
test_exe = executable('mesonsbom_tests',
  sources : files('tests/main.cpp', 'tests/test_json.cpp', 'tests/integration_test.cpp', 'tests/test_transitive_dependencies.cpp', 'tests/test_circular_dependencies.cpp', 'tests/test_pkgconf_integration.cpp', 'tests/test_uuid_generation.cpp', 'tests/test_timestamp.cpp'),
  dependencies : [json_dep, cxxopts_dep, catch2_dep, pkgconf_dep],
  include_directories: pkgconf_inc,
  cpp_args: mesonsbom_version)

test('mesonsbom-tests', test_exe,
  args: ['-r', 'tap'],
  protocol: 'tap')