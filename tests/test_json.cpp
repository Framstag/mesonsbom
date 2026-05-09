#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>
#include "../src/sbom_builder.h"

// Regex for urn:uuid:<UUIDv4> format
static const std::regex serialNumberRegex(
    "^urn:uuid:[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");

// Regex for ISO 8601 UTC timestamp format
static const std::regex timestampRegex(
    "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$");

TEST_CASE("SBOMBuilder creates a valid CycloneDX BOM", "[sbom]") {
    // Initialise builder with a main component
    SBOMBuilder sbom("test-app", "1.0.0");

    // Verify top‑level fields are present
    const auto& bom = sbom.getBom();
    REQUIRE(bom.contains("bomFormat"));
    REQUIRE(bom["bomFormat"] == "CycloneDX");
    REQUIRE(bom.contains("specVersion"));
    REQUIRE(bom["specVersion"] == "1.6");
    REQUIRE(bom.contains("version"));
    REQUIRE(bom["version"] == 1);
    REQUIRE(bom["metadata"]["component"]["type"] == "application");
    REQUIRE(bom["metadata"]["component"]["name"] == "test-app");
    REQUIRE(bom["metadata"]["component"]["version"] == "1.0.0");

    // Add a library component and a dependency relationship
    size_t initialSize = sbom.getComponents().size();
    sbom.addComponent("test-lib", "2.0.0", "library");
    sbom.addDependency("test-app", "test-lib");
    REQUIRE(sbom.getComponents().size() == initialSize + 1);

    // Verify the added component
    const auto& comp = sbom.getComponents().back();
    REQUIRE(comp["type"] == "library");
    REQUIRE(comp["name"] == "test-lib");
    REQUIRE(comp["version"] == "2.0.0");
    // Verify purl is present
    REQUIRE(comp.contains("purl"));
    REQUIRE(comp["purl"] == "pkg:generic/test-lib@2.0.0");

    // Write the BOM to a string and parse it back to JSON to ensure validity
    std::ostringstream oss;
    sbom.writeTo(oss);
    std::string output = oss.str();
    REQUIRE(!output.empty());
    nlohmann::json parsed = nlohmann::json::parse(output);
    REQUIRE(parsed["bomFormat"] == "CycloneDX");
    REQUIRE(parsed["components"].size() >= 2);

    // Verify consolidated dependencies: one entry with dependsOn array
    REQUIRE(parsed.contains("dependencies"));
    REQUIRE(parsed["dependencies"].size() >= 1);
    REQUIRE(parsed["dependencies"][0]["ref"] == "test-app");
    REQUIRE(parsed["dependencies"][0]["dependsOn"].is_array());
    REQUIRE(parsed["dependencies"][0]["dependsOn"][0] == "test-lib");
}

TEST_CASE("SBOMBuilder supports purl, description, and licenses", "[sbom]") {
    SBOMBuilder sbom("pkg-test", "1.0.0");

    // Add a component with description and license
    sbom.addComponent("test-lib", "2.0.0", "library", "A test library", {"MIT"});
    sbom.addDependency("pkg-test", "test-lib");

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    // Check purl on main component
    REQUIRE(parsed["components"][0]["purl"] == "pkg:generic/pkg-test@1.0.0");

    // Check purl, description, and licenses on library component
    bool foundLib = false;
    for (const auto& comp : parsed["components"]) {
        if (comp["name"] == "test-lib") {
            foundLib = true;
            REQUIRE(comp["purl"] == "pkg:generic/test-lib@2.0.0");
            REQUIRE(comp["description"] == "A test library");
            REQUIRE(comp["licenses"].is_array());
            REQUIRE(comp["licenses"][0]["license"]["id"] == "MIT");
        }
    }
    REQUIRE(foundLib);
}

TEST_CASE("SBOM serialNumber is present in writeTo output", "[sbom][serial]") {
    SBOMBuilder sbom("sn-test", "1.0.0");

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    // serialNumber must be present at the top level.
    REQUIRE(parsed.contains("serialNumber"));
    REQUIRE_NOTHROW(parsed["serialNumber"].get<std::string>());
}

TEST_CASE("SBOM serialNumber has valid urn:uuid:UUIDv4 format", "[sbom][serial]") {
    SBOMBuilder sbom("fmt-test", "1.0.0");

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    std::string serial = parsed["serialNumber"].get<std::string>();
    REQUIRE(std::regex_match(serial, serialNumberRegex));
}

TEST_CASE("Same SBOMBuilder instance returns same serialNumber on multiple writeTo calls", "[sbom][serial]") {
    SBOMBuilder sbom("stable-test", "1.0.0");

    std::ostringstream oss1, oss2;
    sbom.writeTo(oss1);
    sbom.writeTo(oss2);

    nlohmann::json parsed1 = nlohmann::json::parse(oss1.str());
    nlohmann::json parsed2 = nlohmann::json::parse(oss2.str());

    REQUIRE(parsed1["serialNumber"] == parsed2["serialNumber"]);
}

TEST_CASE("Different SBOMBuilder instances produce different serialNumbers", "[sbom][serial]") {
    SBOMBuilder sbom1("instance-a", "1.0.0");
    SBOMBuilder sbom2("instance-b", "1.0.0");

    std::ostringstream oss1, oss2;
    sbom1.writeTo(oss1);
    sbom2.writeTo(oss2);

    nlohmann::json parsed1 = nlohmann::json::parse(oss1.str());
    nlohmann::json parsed2 = nlohmann::json::parse(oss2.str());

    REQUIRE(parsed1["serialNumber"] != parsed2["serialNumber"]);
}

TEST_CASE("SBOM metadata.timestamp is present in writeTo output", "[sbom][timestamp]") {
    SBOMBuilder sbom("ts-test", "1.0.0");

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    // metadata.timestamp must be present.
    REQUIRE(parsed.contains("metadata"));
    REQUIRE(parsed["metadata"].contains("timestamp"));
    REQUIRE_NOTHROW(parsed["metadata"]["timestamp"].get<std::string>());
}

TEST_CASE("SBOM metadata.timestamp has valid ISO 8601 UTC format", "[sbom][timestamp]") {
    SBOMBuilder sbom("fmt-test", "1.0.0");

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    std::string ts = parsed["metadata"]["timestamp"].get<std::string>();
    REQUIRE(std::regex_match(ts, timestampRegex));
}

TEST_CASE("Same SBOMBuilder instance returns same timestamp on multiple writeTo calls", "[sbom][timestamp]") {
    SBOMBuilder sbom("stable-test", "1.0.0");

    std::ostringstream oss1, oss2;
    sbom.writeTo(oss1);
    sbom.writeTo(oss2);

    nlohmann::json parsed1 = nlohmann::json::parse(oss1.str());
    nlohmann::json parsed2 = nlohmann::json::parse(oss2.str());

    REQUIRE(parsed1["metadata"]["timestamp"] == parsed2["metadata"]["timestamp"]);
}

TEST_CASE("metadata.timestamp is distinct from serialNumber", "[sbom][timestamp]") {
    // Ensure timestamp is not accidentally set to the serial number or vice versa.
    SBOMBuilder sbom("distinct-test", "1.0.0");

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    std::string serial = parsed["serialNumber"].get<std::string>();
    std::string ts = parsed["metadata"]["timestamp"].get<std::string>();

    REQUIRE(serial != ts);
    // Serial starts with urn:uuid:, timestamp should not.
    REQUIRE(serial.find("urn:uuid:") == 0);
    REQUIRE(ts.find("urn:uuid:") == std::string::npos);
}

TEST_CASE("SBOMBuilder addTool and setLicenses", "[sbom]") {
    SBOMBuilder sbom("tool-test", "2.0.0");

    sbom.addTool({
        .name = "mesonsbom",
        .version = "2.0.0",
        .description = "CycloneDX SBOM generator from Meson build metadata",
        .supplierName = "Tim Teulings",
        .homepageUrl = "https://github.com/Framstag/mesonsbom",
        .vcsUrl = "git+https://github.com/Framstag/mesonsbom.git",
        .licenses = {"GPL-3.0-or-later"}
    });
    sbom.setLicenses({"MIT"});

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    // Check tools.components section (new CycloneDX 1.5+ format)
    REQUIRE(parsed.contains("metadata"));
    REQUIRE(parsed["metadata"].contains("tools"));
    REQUIRE(parsed["metadata"]["tools"].is_object());
    REQUIRE(parsed["metadata"]["tools"].contains("components"));
    REQUIRE(parsed["metadata"]["tools"]["components"].is_array());
    REQUIRE(parsed["metadata"]["tools"]["components"][0]["name"] == "mesonsbom");
    REQUIRE(parsed["metadata"]["tools"]["components"][0]["version"] == "2.0.0");
    REQUIRE(parsed["metadata"]["tools"]["components"][0]["type"] == "application");
    REQUIRE(parsed["metadata"]["tools"]["components"][0]["bom-ref"] == "tool-mesonsbom");

    // Check license on metadata component
    REQUIRE(parsed["metadata"]["component"].contains("licenses"));
    REQUIRE(parsed["metadata"]["component"]["licenses"].is_array());
    REQUIRE(parsed["metadata"]["component"]["licenses"][0]["license"]["id"] == "MIT");
}

TEST_CASE("Tool ordering - mesonsbom is always first", "[sbom]") {
    SBOMBuilder sbom("order-test", "1.0.0");

    // Add mesonsbom first
    sbom.addTool({
        .name = "mesonsbom",
        .version = "1.0.0",
        .description = "Primary SBOM generator"
    });
    // Add another tool second
    sbom.addTool({
        .name = "other-tool",
        .version = "0.5.0",
        .description = "Secondary tool"
    });

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    REQUIRE(parsed["metadata"]["tools"]["components"].size() == 2);
    REQUIRE(parsed["metadata"]["tools"]["components"][0]["name"] == "mesonsbom");
    REQUIRE(parsed["metadata"]["tools"]["components"][1]["name"] == "other-tool");
}

TEST_CASE("Tool ordering - mesonsbom, meson, backend all present", "[sbom]") {
    SBOMBuilder sbom("multi-tool-test", "1.0.0");

    // Add mesonsbom first
    sbom.addTool({
        .name = "mesonsbom",
        .version = "1.0.0",
        .description = "Primary SBOM generator"
    });
    // Add meson second
    sbom.addTool({
        .name = "meson",
        .version = "1.11.1",
        .description = "Build system"
    });
    // Add backend third
    sbom.addTool({
        .name = "ninja",
        .version = "unknown",
        .description = "Build backend"
    });

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());

    REQUIRE(parsed["metadata"]["tools"]["components"].size() == 3);
    REQUIRE(parsed["metadata"]["tools"]["components"][0]["name"] == "mesonsbom");
    REQUIRE(parsed["metadata"]["tools"]["components"][1]["name"] == "meson");
    REQUIRE(parsed["metadata"]["tools"]["components"][1]["version"] == "1.11.1");
    REQUIRE(parsed["metadata"]["tools"]["components"][2]["name"] == "ninja");
    REQUIRE(parsed["metadata"]["tools"]["components"][2]["version"] == "unknown");
    // Verify correct purl for unknown version
    REQUIRE(parsed["metadata"]["tools"]["components"][2]["purl"] == "pkg:generic/ninja@unknown");
    REQUIRE(parsed["metadata"]["tools"]["components"][2]["bom-ref"] == "tool-ninja");
}

TEST_CASE("Tool component has all required fields", "[sbom]") {
    SBOMBuilder sbom("fields-test", "1.0.0");

    sbom.addTool({
        .name = "mesonsbom",
        .version = "1.1.0",
        .description = "CycloneDX SBOM generator from Meson build metadata",
        .supplierName = "Tim Teulings",
        .homepageUrl = "https://github.com/Framstag/mesonsbom",
        .vcsUrl = "git+https://github.com/Framstag/mesonsbom.git",
        .licenses = {"GPL-3.0-or-later"}
    });

    std::ostringstream oss;
    sbom.writeTo(oss);
    nlohmann::json parsed = nlohmann::json::parse(oss.str());
    const auto& tool = parsed["metadata"]["tools"]["components"][0];

    // Core fields
    REQUIRE(tool["type"] == "application");
    REQUIRE(tool["name"] == "mesonsbom");
    REQUIRE(tool["version"] == "1.1.0");
    REQUIRE(tool["bom-ref"] == "tool-mesonsbom");
    REQUIRE(tool["purl"] == "pkg:generic/mesonsbom@1.1.0");
    REQUIRE(tool["description"] == "CycloneDX SBOM generator from Meson build metadata");

    // Supplier
    REQUIRE(tool.contains("supplier"));
    REQUIRE(tool["supplier"]["name"] == "Tim Teulings");

    // Licenses
    REQUIRE(tool.contains("licenses"));
    REQUIRE(tool["licenses"].is_array());
    REQUIRE(tool["licenses"][0]["license"]["id"] == "GPL-3.0-or-later");

    // External references
    REQUIRE(tool.contains("externalReferences"));
    REQUIRE(tool["externalReferences"].is_array());
    REQUIRE(tool["externalReferences"].size() == 2);
    REQUIRE(tool["externalReferences"][0]["type"] == "website");
    REQUIRE(tool["externalReferences"][0]["url"] == "https://github.com/Framstag/mesonsbom");
    REQUIRE(tool["externalReferences"][1]["type"] == "vcs");
    REQUIRE(tool["externalReferences"][1]["url"] == "git+https://github.com/Framstag/mesonsbom.git");
}
