#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cstdlib>

namespace fs = std::filesystem;

TEST_CASE("Transitive Dependency: Nested chain (Project -> A -> B -> C)", "[transitive]") {
    // Setup a temporary build directory
    fs::path tempBuildDir = fs::temp_directory_path() / "mesonsbom_test_nested";
    fs::create_directories(tempBuildDir / "meson-info" / "subprojects" / "A" / "meson-info");
    fs::create_directories(tempBuildDir / "meson-info" / "subprojects" / "B" / "meson-info");
    fs::create_directories(tempBuildDir / "meson-info" / "subprojects" / "C" / "meson-info");

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

    // 4. Mock dependencies for B (B -> C)
    std::ofstream bDeps(tempBuildDir / "meson-info" / "subprojects" / "B" / "intro-dependencies.json");
    bDeps << R"([
        { "name": "C", "version": "3.0.0", "type": "library" }
    ])";
    bDeps.close();

    // 5. Mock dependencies for C (C has no deps)
    std::ofstream cDeps(tempBuildDir / "meson-info" / "subprojects" / "C" / "intro-dependencies.json");
    cDeps << "[]";
    cDeps.close();

    // We need the binary to be available. 
    // Since we are running tests in a CI/controlled environment, we assume 
    // the generator is compiled and in the path or we use a relative path.
    // For the sake of this test, we'll assume 'mesonsbom' is the executable.
    // In a real test, we'd build the project first or use the path to the build artifact.
    
    // For this task, I'll simulate the logic within the test if I can't easily call the binary,
    // BUT the requirement is to "Add integration tests". 
    // So I will attempt to call the actual binary. 
    // I'll assume the test runner is running from the build directory or I know where the binary is.
    
    // Note: To make this work in this environment, I'll assume 'mesonsbom' is in the current path or relative.
    // Actually, a better way is to use the path provided by the environment.
    // I'll try to find the binary.
    
    std::string exePath = "./mesonsbom";
    if (!fs::exists(exePath)) {
        // If not in current dir, check if it's in the current dir's parent (common in build dirs)
        exePath = "../mesonsbom";
    }

    // For the purpose of the exercise, since I cannot guarantee the binary is built/available in the current shell,
    // I will write a test that uses the logic of the generator if necessary, but the goal is an INTEGRATION test.
    // I'll proceed with the intention that this test will be run by the test runner.

    std::string command = exePath + " --build-dir " + tempBuildDir.string() + " --output " + (tempBuildDir / "sbom.json").string();
    int ret = std::system(command.c_str());
    
    // We expect the command to work. If it fails here due to missing binary in this specific agent session,
    // the test will fail, which is correct for a real integration test.
    REQUIRE(ret == 0);

    // Verify sbom.json
    fs::path sbomPath = tempBuildDir / "sbom.json";
    REQUIRE(fs::exists(sbomPath));

    std::ifstream in(sbomPath);
    nlohmann::json sbom;
    in >> sbom;

    // Verify all components are present
    REQUIRE(sbom.contains("components"));
    
    auto checkComponent = [&](const std::string& name, const std::string& version) {
        bool found = false;
        for (const auto& comp : sbom["components"]) {
            if (comp["name"] == name && comp["version"] == version) {
                found = true;
                break;
            }
        }
        return found;
    };

    REQUIRE(checkComponent("A", "1.0.0"));
    REQUIRE(checkComponent("B", "2.0.0"));
    REQUIRE(checkComponent("C", "3.0.0"));

    // Verify dependency edges
    // Project -> A
    // A -> B
    // B -> C
    
    auto checkDependency = [&](const std::string& from, const std::string& to) {
        bool found = false;
        for (const auto& dep : sbom["dependencies"]) {
            if (dep["ref"] == from && dep["dependsOn"].is_array()) {
                for (const auto& depRef : dep["dependsOn"]) {
                    if (depRef == to) {
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
        }
        return found;
    };

    REQUIRE(checkDependency("test-project", "A"));
    REQUIRE(checkDependency("A", "B"));
    REQUIRE(checkDependency("B", "C"));

    // Cleanup
    fs::remove_all(tempBuildDir);
}
