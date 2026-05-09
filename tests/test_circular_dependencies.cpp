 #include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cstdlib>

namespace fs = std::filesystem;

TEST_CASE("Circular Dependency: A -> B -> A", "[circular]") {
    // Setup a temporary build directory
    fs::path tempBuildDir = fs::temp_directory_path() / "mesonsbom_test_circular";
    fs::create_directories(tempBuildDir / "meson-info" / "subprojects" / "A" / "meson-info");
    fs::create_directories(tempBuildDir / "meson-info" / "subprojects" / "B" / "meson-info");

    // 1. Mock intro-projectinfo.json
    std::ofstream projInfo(tempBuildDir / "meson-info" / "intro-projectinfo.json");
    projInfo << R"({
        "name": "test-project",
        "version": "1.0.0"
    })";
    projInfo.close();

    // 2. Mock direct dependencies for Project (Project -> A)
    std::ofstream projDeps(tempBuildDir / "meson-info" / "intro-dependencies.json");
    projDeps << R"([
        { "name": "A", "version": "1.0.0", "type": "library" }
    ])";
    projDeps.close();

    // 3. Mock dependencies for A (A -> B)
    std::ofstream aDeps(tempBuildDir / "meson-info" / "subprojects" / "A" / "intro-dependencies.json");
    aDeps << R"([
        { "name": "B", "version": "2.0.0", "type": "library" }
    ])";
    aDeps.close();

    // 4. Mock dependencies for B (B -> A) - The Circle!
    std::ofstream bDeps(tempBuildDir / "meson-info" / "subprojects" / "B" / "intro-dependencies.json");
    bDeps << R"([
        { "name": "A", "version": "1.0.0", "type": "library" }
    ])";
    bDeps.close();

    // Use a shell command to run the generator and redirect stderr to a file to check for the warning.
    std::string exePath = "./mesonsbom";
    if (!fs::exists(exePath)) {
        exePath = "../mesonsbom";
    }

    std::string errorLog = (tempBuildDir / "error.log").string();
    std::string command = exePath + " --build-dir " + tempBuildDir.string() + " --output " + (tempBuildDir / "sbom.json").string() + " 2> " + errorLog;

    int ret = std::system(command.c_str());

    // Verify the generator didn't crash and completed.
    REQUIRE(ret == 0);

    // Verify sbom.json was created.
    fs::path sbomPath = tempBuildDir / "sbom.json";
    REQUIRE(fs::exists(sbomPath));

    // Load and check essential fields.
    std::ifstream in(sbomPath);
    nlohmann::json sbom;
    in >> sbom;

    // Verify both A and B are in the components list (and only once).
    int countA = 0;
    int countB = 0;
    for (const auto& comp : sbom["components"]) {
        if (comp["name"] == "A") countA++;
        if (comp["name"] == "B") countB++;
    }
    REQUIRE(countA == 1);
    REQUIRE(countB == 1);

    // Verify the warning message was printed to stderr.
    std::ifstream errIn(errorLog);
    std::string line;
    bool warningFound = false;
    while (std::getline(errIn, line)) {
        if (line.find("Warning: Circular dependency detected") != std::string::npos) {
            warningFound = true;
            break;
        }
    }
    REQUIRE(warningFound);

    // Cleanup
    fs::remove_all(tempBuildDir);
}
