#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cxxopts.hpp>
#include <deque>
#include <vector>
#include <string>
#include <optional>
#include "sbom_builder.h"
#include "pkg_config_wrapper.h"

namespace fs = std::filesystem;

/**
 * Result of a dependency load operation.
 */
struct DependencyResult {
    bool found;
    nlohmann::json dependencies;
};

/**
 * Helper to load dependencies for a specific component.
 * Priority: 1. Meson subproject introspection -> 2. pkg-config library.
 */
DependencyResult loadDependencies(const std::string& buildDir, const std::string& componentName, PkgConfigWrapper& pkgConf, bool isMainProject = false) {
    // 1. Try Meson introspection for subprojects.
    fs::path path;
    if (isMainProject) {
        path = fs::path(buildDir) / "meson-info" / "intro-dependencies.json";
    } else {
        path = fs::path(buildDir) / "meson-info" / "subprojects" / componentName / "intro-dependencies.json";
    }

    std::ifstream in(path);
    if (in) {
        nlohmann::json j;
        in >> j;
        return {true, j};
    }

    // 2. Fallback: Use PkgConfigWrapper to query system dependencies.
    bool foundInPkgConfig = false;
    // Retrieve dependencies of the package
    std::vector<std::string> deps = pkgConf.getDependencies(componentName, foundInPkgConfig);
    
    if (foundInPkgConfig) {
        nlohmann::json jsonDeps = nlohmann::json::array();
        for (const auto& depName : deps) {
            nlohmann::json dep;
            dep["name"] = depName;
            dep["version"] = "unknown";
            dep["type"] = "library";
            // Try to get the description of each child dependency via pkg-config
            bool childFound = false;
            PackageInfo childInfo = pkgConf.getPackageInfo(depName, childFound);
            if (childFound && !childInfo.description.empty()) {
                dep["description"] = childInfo.description;
            } else {
                dep["description"] = "";
            }
            jsonDeps.push_back(dep);
        }
        return {true, jsonDeps};
    }

    // Not found in either source.
    return {false, nlohmann::json::array()};
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("mesonsbom", "Generate CycloneDX SBOM from Meson build metadata");
    options.add_options()
        ("b,build-dir", "Path to Meson build directory", cxxopts::value<std::string>())
        ("o,output", "Path to output SBOM file", cxxopts::value<std::string>()->default_value("sbom.json"))
        ("t,target", "Target name (from intro-targets.json) to generate SBOM for", cxxopts::value<std::string>())
        ("v,version", "Print version and exit")
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    // Handle --version before any other options
    if (result.count("version")) {
        std::cout << "mesonsbom version " << MESONSBOM_VERSION << std::endl;
        return 0;
    }

    if (result.count("help") || !result.count("build-dir")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    std::string buildDir = result["build-dir"].as<std::string>();
    std::string outputFilePath = result["output"].as<std::string>();
    bool hasTarget = result.count("target");
    std::string targetName = hasTarget ? result["target"].as<std::string>() : "";

    // Load project info to get the name and version of the main component
    fs::path projectInfoPath = fs::path(buildDir) / "meson-info" / "intro-projectinfo.json";
    std::ifstream projIn(projectInfoPath);
    if (!projIn) {
        std::cerr << "Error: Failed to open file: " << projectInfoPath.string() << std::endl;
        return 1;
    }
    nlohmann::json projectInfo;
    projIn >> projectInfo;

    std::string projName = projectInfo.value("name", projectInfo.value("descriptive_name", "unknown"));
    std::string projVersion = projectInfo.value("version", "0.0.0");

    // When --target is specified, load intro-targets.json and use the target's name
    // as the main component name (Decision 1, Decision 2 from design).
    std::vector<std::string> targetDependencyNames; // empty means no filtering (full project)
    if (hasTarget) {
        fs::path targetsPath = fs::path(buildDir) / "meson-info" / "intro-targets.json";
        std::ifstream targetsIn(targetsPath);
        if (!targetsIn) {
            std::cerr << "Error: Failed to open file: " << targetsPath.string() << std::endl;
            return 1;
        }
        nlohmann::json targetsJson;
        targetsIn >> targetsJson;

        // Find the target by name (Decision 1: match by name, not id)
        bool found = false;
        std::vector<std::string> availableNames;
        for (const auto& t : targetsJson) {
            std::string tgtName = t.value("name", "");
            availableNames.push_back(tgtName);
            if (tgtName == targetName) {
                // Use the target's name as the main component
                projName = tgtName;
                // Collect the target's dependency names for filtering (Decision 3)
                if (t.contains("dependencies") && t["dependencies"].is_array()) {
                    for (const auto& dep : t["dependencies"]) {
                        targetDependencyNames.push_back(dep.get<std::string>());
                    }
                }
                found = true;
                break;
            }
        }

        if (!found) {
            std::cerr << "Error: Target '" << targetName << "' not found in meson-info/intro-targets.json.\n"
                      << "Available targets:";
            for (const auto& name : availableNames) {
                std::cerr << " " << name;
            }
            std::cerr << std::endl;
            return 1;
        }
    }

    SBOMBuilder sbom(projName, projVersion);
    PkgConfigWrapper pkgConf;

    // Populate metadata.tools with mesonsbom identity using embedded version
    sbom.setTools("mesonsbom", MESONSBOM_VERSION);

    // 3.2: Extract license array from intro-projectinfo.json and pass to setLicenses
    std::vector<std::string> projectLicenses;
    if (projectInfo.contains("license") && projectInfo["license"].is_array()) {
        for (const auto& lic : projectInfo["license"]) {
            projectLicenses.push_back(lic.get<std::string>());
        }
    }
    sbom.setLicenses(projectLicenses);

    // To detect cycles and manage traversal, we use a Node that tracks the path taken.
    struct Node {
        std::string name;
        std::vector<std::string> path;
    };
    std::deque<Node> queue;

    // 1. Process direct dependencies of the main project
    auto [mainFound, directDeps] = loadDependencies(buildDir, "", pkgConf, true);
    if (mainFound && directDeps.is_array()) {
        for (const auto& dep : directDeps) {
            std::string depType = dep.value("type", "");
            if (depType == "optional") continue;
            
            std::string depName = dep.value("name", "");
            std::string depVersion = dep.value("version", "0.0.0");
            std::string depDescription = dep.value("description", "");

            // When --target is specified, only include dependencies that belong to
            // the requested target (Decision 3 from design).
            if (!targetDependencyNames.empty()) {
                bool matchesTarget = false;
                for (const auto& tgtDep : targetDependencyNames) {
                    if (tgtDep == depName) {
                        matchesTarget = true;
                        break;
                    }
                }
                if (!matchesTarget) continue;
            }
            
            if (!depName.empty()) {
                if (sbom.addComponent(depName, depVersion, "library", depDescription)) {
                    queue.push_back({depName, {projName, depName}});
                }
                sbom.addDependency(projName, depName);
            }
        }
    }

    // 2. Recursively process transitive dependencies using BFS
    while (!queue.empty()) {
        Node current = queue.front();
        queue.pop_front();

        auto [found, transitiveDeps] = loadDependencies(buildDir, current.name, pkgConf);
        if (!found) {
            std::cerr << "Warning: Could not resolve dependencies for " << current.name 
                      << " via Meson subprojects or pkg-config." << std::endl;
            continue;
        }

        if (transitiveDeps.is_array()) {
            for (const auto& dep : transitiveDeps) {
                std::string depType = dep.value("type", "");
                if (depType == "optional") continue;

                std::string depName = dep.value("name", "");
                std::string depVersion = dep.value("version", "0.0.0");
                std::string depDescription = dep.value("description", "");

                if (!depName.empty()) {
                    // Circularity Detection: check if depName is already an ancestor in the current path
                    for (const auto& ancestor : current.path) {
                        if (ancestor == depName) {
                            std::cerr << "Warning: Circular dependency detected: " << depName 
                                      << " is an ancestor of itself in the current path." << std::endl;
                            break;
                        }
                    }

                    if (sbom.addComponent(depName, depVersion, "library", depDescription)) {
                        std::vector<std::string> nextPath = current.path;
                        nextPath.push_back(depName);
                        queue.push_back({depName, nextPath});
                    }
                    // 3.4: Consolidated addDependency works correctly - each call accumulates
                    // into the same depMap entry for the same 'from' component
                    sbom.addDependency(current.name, depName);
                }
            }
        }
    }

    // Write the final result to the output file
    try {
        std::ofstream out(outputFilePath);
        if (!out) {
            throw std::runtime_error("Could not open output file for writing: " + outputFilePath);
        }
        sbom.writeTo(out);
        // Use stderr for informational output so it does not interfere with TAP protocol
        std::cerr << "SBOM written to " << outputFilePath << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}