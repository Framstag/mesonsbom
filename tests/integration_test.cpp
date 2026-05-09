#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

TEST_CASE("Integration: generate SBOM from this project's build directory", "[integration]") {
    // The test runs from the build directory where the binary is located.
    std::filesystem::path buildDir = std::filesystem::current_path();
    std::filesystem::path exe = buildDir / "mesonsbom";
    REQUIRE(std::filesystem::exists(exe));

    // Execute the generator pointing at the same build directory.
    std::string command = exe.string() + " --build-dir " + buildDir.string();
    int ret = std::system(command.c_str());
    REQUIRE(ret == 0);

    // Verify sbom.json was created.
    std::filesystem::path sbomPath = buildDir / "sbom.json";
    REQUIRE(std::filesystem::exists(sbomPath));

    // Load and check essential fields.
    std::ifstream in(sbomPath);
    REQUIRE(in.good());
    nlohmann::json sbom;
    in >> sbom;
    REQUIRE(sbom.contains("bomFormat"));
    REQUIRE(sbom["bomFormat"] == "CycloneDX");
    REQUIRE(sbom.contains("specVersion"));
    REQUIRE(sbom["specVersion"] == "1.6");
    REQUIRE(sbom.contains("metadata"));
    REQUIRE(sbom["metadata"]["component"]["type"] == "application");

    // Ensure at least one library component is present, each library has a version, and every component has a bom-ref and purl.
    bool hasLibrary = false;
    if (sbom.contains("components")) {
        for (const auto& comp : sbom["components"]) {
            // Every component must have a unique bom-ref.
            REQUIRE(comp.contains("bom-ref"));
            REQUIRE(!comp["bom-ref"].get<std::string>().empty());
            // Every component must have a purl
            REQUIRE(comp.contains("purl"));
            REQUIRE(!comp["purl"].get<std::string>().empty());
            if (comp["type"] == "library") {
                hasLibrary = true;
                // Verify version field exists and is non-empty.
                REQUIRE(comp.contains("version"));
                REQUIRE(!comp["version"].get<std::string>().empty());
            }
        }
    }
    REQUIRE(hasLibrary);

    // Verify that dependencies reference the correct bom-ref of the main component.
    if (!sbom["dependencies"].empty()) {
        const auto& dep = sbom["dependencies"][0];
        // The ref should match the bom-ref of the main component (metadata.component.name).
        std::string mainBomRef = sbom["metadata"]["component"]["name"].get<std::string>();
        REQUIRE(dep["ref"] == mainBomRef);
        // Verify consolidated format: dependsOn is an array
        REQUIRE(dep["dependsOn"].is_array());
    }

    // Verify metadata.tools section contains mesonsbom
    if (sbom["metadata"].contains("tools")) {
        const auto& tools = sbom["metadata"]["tools"];
        REQUIRE(tools.is_array());
        REQUIRE(tools[0]["name"] == "mesonsbom");
    }

    // Verify the first component (application) has a bom-ref that matches its name.
    if (sbom.contains("components") && !sbom["components"].empty()) {
        const auto& appComp = sbom["components"][0];
        REQUIRE(appComp["type"] == "application");
        REQUIRE(appComp["name"] == sbom["metadata"]["component"]["name"].get<std::string>());
        REQUIRE(appComp["bom-ref"] == appComp["name"].get<std::string>());
    }

    // Clean up for repeatable runs.
    std::filesystem::remove(sbomPath);
}
