#include <catch2/catch_test_macros.hpp>
#include <string>
#include <fstream>
#include "../src/os_release.h"

// -----------------------------------------------------------------------
// Tests for OsRelease — parsing /etc/os-release format.
// -----------------------------------------------------------------------

TEST_CASE("parseOsRelease parses ID, NAME, VERSION_ID correctly", "[os_release]") {
    std::string content =
        "NAME=\"Arch Linux\"\n"
        "ID=arch\n"
        "PRETTY_NAME=\"Arch Linux\"\n"
        "ID_LIKE=\"arch\"\n"
        "VERSION_ID=rolling\n"
        "BUILD_ID=20240101\n"
        ;

    std::ofstream tmp("/tmp/test_os_release_ok");
    REQUIRE(tmp.is_open());
    tmp << content;
    tmp.close();

    OsRelease release = parseOsRelease("/tmp/test_os_release_ok");
    REQUIRE(release.id == "arch");
    REQUIRE(release.name == "Arch Linux");
    REQUIRE(release.versionId == "rolling");
}

TEST_CASE("parseOsRelease handles missing file gracefully", "[os_release]") {
    OsRelease release = parseOsRelease("/tmp/nonexistent_os_release_xyz");
    REQUIRE(release.id.empty());
    REQUIRE(release.name.empty());
    REQUIRE(release.versionId.empty());
}

TEST_CASE("parseOsRelease handles file without VERSION_ID", "[os_release]") {
    std::string content =
        "ID=debian\n"
        "NAME=\"Debian GNU/Linux\"\n"
        ;

    std::ofstream tmp("/tmp/test_os_release_no_ver");
    REQUIRE(tmp.is_open());
    tmp << content;
    tmp.close();

    OsRelease release = parseOsRelease("/tmp/test_os_release_no_ver");
    REQUIRE(release.id == "debian");
    REQUIRE(release.name == "Debian GNU/Linux");
    REQUIRE(release.versionId.empty());
}

TEST_CASE("parseOsRelease handles quoted and unquoted values", "[os_release]") {
    std::string content =
        "ID=fedora\n"
        "NAME=Fedora\n"       // unquoted
        "VERSION_ID=38\n"      // numeric unquoted
        ;

    std::ofstream tmp("/tmp/test_os_release_unquoted");
    REQUIRE(tmp.is_open());
    tmp << content;
    tmp.close();

    OsRelease release = parseOsRelease("/tmp/test_os_release_unquoted");
    REQUIRE(release.id == "fedora");
    REQUIRE(release.name == "Fedora");
    REQUIRE(release.versionId == "38");
}

TEST_CASE("parseOsRelease handles single-quoted values", "[os_release]") {
    std::string content =
        "ID='ubuntu'\n"
        "NAME='Ubuntu'\n"
        "VERSION_ID='22.04'\n"
        ;

    std::ofstream tmp("/tmp/test_os_release_single_quote");
    REQUIRE(tmp.is_open());
    tmp << content;
    tmp.close();

    OsRelease release = parseOsRelease("/tmp/test_os_release_single_quote");
    REQUIRE(release.id == "ubuntu");
    REQUIRE(release.name == "Ubuntu");
    REQUIRE(release.versionId == "22.04");
}

TEST_CASE("parseOsRelease skips comments and empty lines", "[os_release]") {
    std::string content =
        "# This is a comment\n"
        "\n"
        "ID=void\n"
        "NAME=\"Void Linux\"\n"
        ;

    std::ofstream tmp("/tmp/test_os_release_comment");
    REQUIRE(tmp.is_open());
    tmp << content;
    tmp.close();

    OsRelease release = parseOsRelease("/tmp/test_os_release_comment");
    REQUIRE(release.id == "void");
    REQUIRE(release.name == "Void Linux");
}