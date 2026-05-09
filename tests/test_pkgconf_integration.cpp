#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <vector>
#include <string>

namespace fs = std::filesystem;

// Helper to create a simple .pc file
void createPcFile(const fs::path& dir, const std::string& name, const std::vector<std::string>& deps) {
    std::ofstream pc(dir / (name + ".pc"));
    pc << "Name: " << name << "\n";
    pc << "Description: Mock library " << name << "\n";
    pc << "Version: 1.0.0\n";
    if (!deps.empty()) {
        pc << "Requires: ";
        for (size_t i = 0; i < deps.size(); ++i) {
            pc << deps[i] << (i == deps.size() - 1 ? "" : ", ");
        }
        pc << "\n";
    }
    pc.close();
}

TEST_CASE("PkgConfigIntegration: Transitive system dependencies", "[pkgconf]") {
    // Setup temporary directory for mock .pc files
    fs::path pkgConfigDir = fs::temp_directory_path() / "mesonsbom_pkgconfig_test";
    fs::create_directories(pkgConfigDir);

    // Create mock dependencies: libA -> libB -> libC
    createPcFile(pkgConfigDir, "mocklibA", {"mocklibB"});
    createPcFile(pkgConfigDir, "mocklibB", {"mocklibC"});
    createPcFile(pkgConfigDir, "mocklibC", {});

    // We also need a build directory with intro-dependencies.json that points to mocklibA
    fs::path tempBuildDir = fs::temp_directory_path() / "mesonsbom_pkg_build";
    fs::create_directories(tempBuildDir / "meson-info");
    
    std::ofstream projInfo(tempBuildDir / "meson-info" / "intro-projectinfo.json");
    projInfo << R"({"name": "pkg-test", "version": "1.0.0"})";
    projInfo.close();

    std::ofstream projDeps(tempBuildDir / "meson-info" / "intro-dependencies.json");
    projDeps << R"([{"name": "mocklibA", "version": "1.0.0", "type": "library"}])";
    projDeps.close();

    // Execute the binary with PKG_CONFIG_PATH set
    std::string exePath = "./mesonsbom";
    if (!fs::exists(exePath)) exePath = "../mesonsbom";

    std::string outputFilePath = (tempBuildDir / "sbom.json").string();
    // Use env to set PKG_CONFIG_PATH for the command
    std::string command = "PKG_CONFIG_PATH=" + pkgConfigDir.string() + " " + exePath + " --build-dir " + tempBuildDir.string() + " --output " + outputFilePath;
    
    int ret = std::system(command.c_str());
    REQUIRE(ret == 0);

    // Verify SBOM
    std::ifstream in(outputFilePath);
    nlohmann::json sbom;
    in >> sbom;

    auto checkComponent = [&](const std::string& name) {
        for (const auto& comp : sbom["components"]) {
            if (comp["name"] == name) return true;
        }
        return false;
    };

    REQUIRE(checkComponent("mocklibA"));
    REQUIRE(checkComponent("mocklibB"));
    REQUIRE(checkComponent("mocklibC"));

    fs::remove_all(pkgConfigDir);
    fs::remove_all(tempBuildDir);
}

TEST_CASE("PkgConfigIntegration: Non-existent package warning", "[pkgconf]") {
    fs::path tempBuildDir = fs::temp_directory_path() / "mesonsbom_pkg_missing";
    fs::create_directories(tempBuildDir / "meson-info");
    
    std::ofstream projInfo(tempBuildDir / "meson-info" / "intro-projectinfo.json");
    projInfo << R"({"name": "missing-test", "version": "1.0.0"})";
    projInfo.close();

    std::ofstream projDeps(tempBuildDir / "meson-info" / "intro-dependencies.json");
    projDeps << R"([{"name": "nonexistent-lib", "version": "1.0.0", "type": "library"}])";
    projDeps.close();

    std::string exePath = "./mesonsbom";
    if (!fs::exists(exePath)) exePath = "../mesonsbom";

    std::string errorLog = (tempBuildDir / "error.log").string();
    std::string command = exePath + " --build-dir " + tempBuildDir.string() + " 2> " + errorLog;
    
    int ret = std::system(command.c_str());
    REQUIRE(ret == 0);

    std::ifstream errIn(errorLog);
    std::string line;
    bool warningFound = false;
    while (std::getline(errIn, line)) {
        if (line.find("Warning: Could not resolve dependencies") != std::string::npos) {
            warningFound = true;
            break;
        }
    }
    REQUIRE(warningFound);

    fs::remove_all(tempBuildDir);
}
