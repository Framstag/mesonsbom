#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <random>
#include <ctime>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>

// ---------------------------------------------------------------------------
// UUIDv4 generation helpers
// ---------------------------------------------------------------------------

namespace detail {

// Core UUIDv4 generator: fills 16 random bytes, sets version (4) and variant
// (RFC 4122) nibbles, formats as xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx.
// Takes a caller-provided RNG by reference so tests can use known seeds.
inline std::string makeUuidV4(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, 255);
    uint8_t bytes[16];
    for (int i = 0; i < 16; ++i) {
        bytes[i] = static_cast<uint8_t>(dist(rng));
    }
    // Set version nibble to 4 (UUIDv4).
    bytes[6] = (bytes[6] & 0x0f) | 0x40;
    // Set variant nibble to 10xx (RFC 4122).
    bytes[8] = (bytes[8] & 0x3f) | 0x80;

    static constexpr char hex[] = "0123456789abcdef";
    std::string uuid(36, '\0');
    int j = 0;
    for (int i = 0; i < 16; ++i) {
        uuid[j++] = hex[bytes[i] >> 4];
        uuid[j++] = hex[bytes[i] & 0x0f];
        if (i == 3 || i == 5 || i == 7 || i == 9) uuid[j++] = '-';
    }
    return uuid;
}

} // namespace detail

// Generate a UUIDv4 using a process-wide static std::mt19937.
// The PRNG is seeded once on first call with
//   std::random_device{}() ^ std::time(nullptr)
// The XOR with current time guarantees a different seed per application start
// even on platforms where std::random_device is deterministic (e.g., MinGW).
inline std::string generateUuidV4() {
    static std::mt19937 rng(static_cast<unsigned>(
        std::random_device{}() ^ static_cast<unsigned>(std::time(nullptr))));
    return detail::makeUuidV4(rng);
}

// Overload that uses a caller-provided RNG. Useful for tests that need
// deterministic seeds or to verify seed-variation behavior.
inline std::string generateUuidV4(std::mt19937& rng) {
    return detail::makeUuidV4(rng);
}

// ---------------------------------------------------------------------------
// Timestamp generation helpers
// ---------------------------------------------------------------------------

// Generate an ISO 8601 UTC timestamp string in the format YYYY-MM-DDTHH:mm:ssZ.
// Uses std::time(nullptr) for current time and std::gmtime + std::strftime
// for formatting. Returns the formatted string, or a fallback epoch string
// if gmtime returns nullptr.
inline std::string makeTimestamp() {
    const std::time_t now = std::time(nullptr);
    // gmtime can return nullptr on error (negative time_t, etc.)
    const std::tm* gmt = std::gmtime(&now);
    if (!gmt) {
        return "1970-01-01T00:00:00Z";
    }
    char buf[25]; // enough for "YYYY-MM-DDTHH:mm:ssZ\0"
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmt);
    return std::string(buf);
}

/**
 * Structured metadata for a tool that contributed to BOM creation.
 * Used with addTool() to populate metadata.tools.components per CycloneDX 1.5+.
 */
struct ToolInfo {
    std::string name;
    std::string version;
    std::string type = "application";
    std::string description;
    std::string supplierName;
    std::string homepageUrl;
    std::string vcsUrl;
    std::vector<std::string> licenses; // SPDX license identifiers
};

/**
 * Simple SBOM builder for CycloneDX 1.6 JSON output.
 * It builds a minimal BOM containing the main application component and any
 * runtime library components with dependency relationships.
 */
class SBOMBuilder {
public:
    // Initialise the BOM with the main component name and version.
    // Also generates a random UUIDv4 serial number for the CycloneDX document.
    SBOMBuilder(const std::string& name, const std::string& version)
        : serialNumber_(generateUuidV4())
        , timestamp_(makeTimestamp())
    {
        // Top‑level BOM fields required by CycloneDX.
        bom["bomFormat"] = "CycloneDX";
        bom["specVersion"] = "1.6";
        bom["version"] = 1;

        // Metadata for the main component.
        bom["metadata"]["component"]["type"] = "application";
        bom["metadata"]["component"]["name"] = name;
        bom["metadata"]["component"]["version"] = version;

        // Also add the main component to the components array for convenience.
        nlohmann::json mainComp;
        mainComp["type"] = "application";
        mainComp["name"] = name;
        mainComp["version"] = version;
        // CycloneDX uses a "bom-ref" to uniquely identify components. Use the component name as the reference.
        mainComp["bom-ref"] = name;
        // Add purl for the main component
        mainComp["purl"] = makePurl(name, version);
        components.push_back(mainComp);

        // Map component name to its bom-ref for later dependency construction.
        nameToRef[name] = name;
    }

    // Add a library/component to the SBOM.
    // @param description Optional description (e.g., from pkg-config .pc file).
    //                     Empty string if not available.
    // @param licenses    Optional list of license identifiers (e.g., ["MIT", "Apache-2.0"]).
    //                     Empty list if not available.
    bool addComponent(const std::string& name,
                      const std::string& version,
                      const std::string& type,
                      const std::string& description = "",
                      const std::vector<std::string>& licenses = {}) {
        if (visited.count(name)) {
            return false; // already added
        }
        nlohmann::json comp;
        comp["type"] = type;
        comp["name"] = name;
        comp["version"] = version;
        // Assign a unique bom-ref (use the component name for simplicity).
        comp["bom-ref"] = name;
        // Add purl using the pkg:generic format
        comp["purl"] = makePurl(name, version);
        // Add description if available
        if (!description.empty()) {
            comp["description"] = description;
        }
        // Add licenses if available
        if (!licenses.empty()) {
            nlohmann::json licenseArray = nlohmann::json::array();
            for (const auto& lic : licenses) {
                nlohmann::json licObj;
                licObj["license"]["id"] = lic;
                licenseArray.push_back(licObj);
            }
            comp["licenses"] = licenseArray;
        }
        components.push_back(comp);
        // Store mapping for dependency resolution.
        nameToRef[name] = name;
        visited.insert(name);
        return true;
    }

    // Record a dependency relationship: `from` depends on `to`.
    // Consolidates all dependencies of a single `from` component into one entry.
    void addDependency(const std::string& from, const std::string& to) {
        std::string fromRef = nameToRef.count(from) ? nameToRef[from] : from;
        std::string toRef   = nameToRef.count(to)   ? nameToRef[to]   : to;
        
        // Consolidate: add 'to' to the list of dependencies for 'from'
        depMap[fromRef].push_back(toRef);
    }

    // Add a tool entry to metadata.tools.components per CycloneDX 1.5+.
    // Returns insertion order index (first call = 0, second = 1, ...)
    // mesonsbom should be added first to guarantee it appears first in output.
    size_t addTool(const ToolInfo& info) {
        toolComponents_.push_back(info);
        return toolComponents_.size() - 1;
    }

    // Add license information to metadata.component.licenses.
    // @param licenseIds List of SPDX license identifiers (e.g., ["MIT"]).
    void setLicenses(const std::vector<std::string>& licenseIds) {
        if (licenseIds.empty()) {
            return;
        }
        nlohmann::json licenseArray = nlohmann::json::array();
        for (const auto& lic : licenseIds) {
            if (lic != "unknown") {
                nlohmann::json licObj;
                licObj["license"]["id"] = lic;
                licenseArray.push_back(licObj);
            }
        }
        if (!licenseArray.empty()) {
            bom["metadata"]["component"]["licenses"] = licenseArray;
        }
    }

    // Write the complete BOM JSON to the provided output stream.
    void writeTo(std::ostream& out) const {
        nlohmann::json result = bom;
        // CycloneDX serialNumber: required URN with UUIDv4.
        result["serialNumber"] = std::string("urn:uuid:") + serialNumber_;
        // CycloneDX metadata.timestamp: ISO 8601 UTC generation time.
        result["metadata"]["timestamp"] = timestamp_;
        if (!components.empty()) {
            result["components"] = components;
        }
        // Write tools.components (replaces old flat metadata.tools array)
        writeTools(result);

        // Serialize consolidated dependencies
        if (!depMap.empty()) {
            nlohmann::json depsArray = nlohmann::json::array();
            for (const auto& [fromRef, toRefs] : depMap) {
                nlohmann::json dep;
                dep["ref"] = fromRef;
                dep["dependsOn"] = nlohmann::json::array();
                for (const auto& toRef : toRefs) {
                    dep["dependsOn"].push_back(toRef);
                }
                depsArray.push_back(dep);
            }
            result["dependencies"] = depsArray;
        }
        out << result.dump(2);
    }

    // Accessors used by unit tests.
    const nlohmann::json& getBom() const { return bom; }
    const std::vector<nlohmann::json>& getComponents() const { return components; }

    bool hasComponent(const std::string& name) const { return visited.count(name) > 0; }

    // Access the generated serial number (used by tests).
    const std::string& getSerialNumber() const { return serialNumber_; }

    // Access the captured timestamp (used by tests).
    const std::string& getTimestamp() const { return timestamp_; }

private:
    std::string serialNumber_;              // UUIDv4 serial number, generated once in constructor
    std::string timestamp_;                 // ISO 8601 UTC timestamp, captured once in constructor
    nlohmann::json bom;                     // top‑level BOM object
    std::vector<nlohmann::json> components; // list of component objects
    std::unordered_map<std::string, std::string> nameToRef; // map component name → bom-ref
    std::unordered_set<std::string> visited; // set of components already added to BOM
    std::vector<ToolInfo> toolComponents_; // ordered list of tool entries
    std::unordered_map<std::string, std::vector<std::string>> depMap; // consolidated dependency map

    // Serialize all tool entries into metadata.tools.components (new CycloneDX 1.5+ format).
    // Replaces the deprecated flat metadata.tools array entirely.
    void writeTools(nlohmann::json& target) const {
        if (toolComponents_.empty()) return;
        nlohmann::json components = nlohmann::json::array();
        for (const auto& tool : toolComponents_) {
            nlohmann::json comp;
            comp["type"] = tool.type;
            comp["name"] = tool.name;
            comp["version"] = tool.version;
            comp["bom-ref"] = "tool-" + tool.name;
            comp["purl"] = makePurl(tool.name, tool.version);
            if (!tool.description.empty()) {
                comp["description"] = tool.description;
            }
            if (!tool.supplierName.empty()) {
                comp["supplier"]["name"] = tool.supplierName;
            }
            if (!tool.licenses.empty()) {
                nlohmann::json licArray = nlohmann::json::array();
                for (const auto& lic : tool.licenses) {
                    nlohmann::json licObj;
                    licObj["license"]["id"] = lic;
                    licArray.push_back(licObj);
                }
                comp["licenses"] = licArray;
            }
            if (!tool.homepageUrl.empty() || !tool.vcsUrl.empty()) {
                nlohmann::json refs = nlohmann::json::array();
                if (!tool.homepageUrl.empty()) {
                    nlohmann::json ref;
                    ref["type"] = "website";
                    ref["url"] = tool.homepageUrl;
                    refs.push_back(ref);
                }
                if (!tool.vcsUrl.empty()) {
                    nlohmann::json ref;
                    ref["type"] = "vcs";
                    ref["url"] = tool.vcsUrl;
                    refs.push_back(ref);
                }
                comp["externalReferences"] = refs;
            }
            components.push_back(comp);
        }
        target["metadata"]["tools"]["components"] = components;
    }

    // Generate a purl using the pkg:generic format.
    // Format: pkg:generic/<lowercase-name>@<version>
    static std::string makePurl(const std::string& name, const std::string& version) {
        std::string lowerName;
        for (char c : name) {
            lowerName += std::tolower(static_cast<unsigned char>(c));
        }
        return "pkg:generic/" + lowerName + "@" + version;
    }
};
