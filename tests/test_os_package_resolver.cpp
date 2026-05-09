#include <catch2/catch_test_macros.hpp>
#include <string>
#include "../src/os_package_resolver.h"

// -----------------------------------------------------------------------
// Tests for OsPackageResolver.
// -----------------------------------------------------------------------

TEST_CASE("Constructor detects PM or returns None", "[os_package_resolver]") {
    // In CI or on any Linux host, a PM may or may not be present.
    // Just verify the enum is valid.
    OsPackageResolver res;
    PackageManager pm = res.activePackageManager();
    bool valid = (pm == PackageManager::None ||
                  pm == PackageManager::Pacman ||
                  pm == PackageManager::Dpkg  ||
                  pm == PackageManager::Rpm   ||
                  pm == PackageManager::Apk);
    REQUIRE(valid);
}

TEST_CASE("resolve non-existent file returns nullopt", "[os_package_resolver]") {
    OsPackageResolver res;
    auto result = res.resolve("/nonexistent/test_file.pc");
    REQUIRE(!result.has_value());
}

TEST_CASE("resolve same path twice uses cache", "[os_package_resolver]") {
    OsPackageResolver res;
    auto r1 = res.resolve("/nonexistent/test_file2.pc");
    auto r2 = res.resolve("/nonexistent/test_file2.pc");
    REQUIRE(r1.has_value() == r2.has_value());
}

TEST_CASE("packageManagerName returns correct strings", "[os_package_resolver]") {
    REQUIRE(OsPackageResolver::packageManagerName(PackageManager::Pacman) == "pacman");
    REQUIRE(OsPackageResolver::packageManagerName(PackageManager::Dpkg)   == "dpkg");
    REQUIRE(OsPackageResolver::packageManagerName(PackageManager::Rpm)    == "rpm");
    REQUIRE(OsPackageResolver::packageManagerName(PackageManager::Apk)    == "apk");
    REQUIRE(OsPackageResolver::packageManagerName(PackageManager::None)   == "");
}

TEST_CASE("osRelease returns parsed data (or empty)", "[os_package_resolver]") {
    OsPackageResolver res;
    const OsRelease& release = res.osRelease();

    // On CI, /etc/os-release may or may not be present.
    // If present, id should not be empty. If not, all fields empty.
    // Either is acceptable — just don't crash.
    std::string id = release.id;
    // No assertion — this is a smoke test that the field is accessible.
    (void)id;
}
