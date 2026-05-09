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
#include "os_package_resolver.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

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
    if (!isMainProject) {
        fs::path subPath = fs::path(buildDir) / "meson-info" / "subprojects" / componentName / "intro-dependencies.json";
        std::ifstream subIn(subPath);
        if (subIn) {
            nlohmann::json j;
            subIn >> j;
            return {true, j};
        }

        // 1b. Check if the component is listed in the main intro-dependencies.json
        // with a "dependencies" field (Meson-discovered children, e.g., Qt5Core's
        // sub-deps like Qt5Svg that pkg-config may not list via Requires).
        fs::path mainDepsPath = fs::path(buildDir) / "meson-info" / "intro-dependencies.json";
        std::ifstream mainIn(mainDepsPath);
        if (mainIn) {
            nlohmann::json mainDeps;
            mainIn >> mainDeps;
            if (mainDeps.is_array()) {
                for (const auto& entry : mainDeps) {
                    if (entry.value("name", "") == componentName) {
                        if (entry.contains("dependencies") && entry["dependencies"].is_array()) {
                            nlohmann::json result = nlohmann::json::array();
                            for (const auto& child : entry["dependencies"]) {
                                std::string childName = child.get<std::string>();
                                // Don't include self-references (Qt5Core -> Qt5Core)
                                if (childName == componentName) continue;
                                nlohmann::json childDep;
                                childDep["name"] = childName;
                                childDep["version"] = "unknown";
                                childDep["type"] = "pkgconfig";
                                result.push_back(childDep);
                            }
                            return {true, result};
                        }
                        break;
                    }
                }
            }
        }
    } else {
        // Main project: read intro-dependencies.json directly
        fs::path path = fs::path(buildDir) / "meson-info" / "intro-dependencies.json";
        std::ifstream in(path);
        if (in) {
            nlohmann::json j;
            in >> j;
            return {true, j};
        }
    }

    // 2. Fallback: Use PkgConfigWrapper to query system dependencies.
    bool foundInPkgConfig = false;
    std::vector<std::string> deps = pkgConf.getDependencies(componentName, foundInPkgConfig);

    if (foundInPkgConfig) {
        nlohmann::json jsonDeps = nlohmann::json::array();
        for (const auto& depName : deps) {
            nlohmann::json dep;
            dep["name"] = depName;
            dep["type"] = "pkgconfig";
            bool childFound = false;
            PackageInfo childInfo = pkgConf.getPackageInfo(depName, childFound);
            dep["version"] = (childFound && !childInfo.version.empty()) ? childInfo.version : "unknown";
            dep["description"] = (childFound && !childInfo.description.empty()) ? childInfo.description : "";
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

    // Read meson-info.json to get the Meson version that built this project.
    // If missing or corrupt, warn and continue without Meson tool metadata.
    fs::path mesonInfoPath = fs::path(buildDir) / "meson-info" / "meson-info.json";
    std::string mesonVersion;
    try {
        std::ifstream mesonIn(mesonInfoPath);
        if (mesonIn) {
            nlohmann::json mesonInfo;
            mesonIn >> mesonInfo;
            mesonVersion = mesonInfo["meson_version"]["full"];
        } else {
            std::cerr << "Warning: Could not open " << mesonInfoPath.string() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not parse meson-info.json: " << e.what() << std::endl;
    }

    // Read intro-buildoptions.json to detect the Meson backend build tool.
    // If missing or backend is unknown, silently skip - this is a minor enrichment.
    fs::path optionsPath = fs::path(buildDir) / "meson-info" / "intro-buildoptions.json";
    std::string backendName;
    {
        std::ifstream optIn(optionsPath);
        if (optIn) {
            nlohmann::json options;
            optIn >> options;
            for (const auto& opt : options) {
                if (opt.value("name", "") == "backend") {
                    backendName = opt.value("value", "");
                    break;
                }
            }
        }
    }
    // Load intro-targets.json to determine which deps are actually used.
    // This filters out Meson-discovered deps that no target references
    // (e.g., Qt6Core discovered in intro-dependencies.json but only Qt5Core
    // used in targets). When targetDependencyNames is non-empty, Phase 1
    // only collects deps whose names appear in this set.
    std::vector<std::string> targetDependencyNames;
    {
        fs::path targetsPath = fs::path(buildDir) / "meson-info" / "intro-targets.json";
        std::ifstream targetsIn(targetsPath);
        if (targetsIn) {
            nlohmann::json targetsJson;
            targetsIn >> targetsJson;

            if (hasTarget) {
                // --target mode: find specific target, use its deps as filter
                bool found = false;
                std::vector<std::string> availableNames;
                for (const auto& t : targetsJson) {
                    std::string tgtName = t.value("name", "");
                    availableNames.push_back(tgtName);
                    if (tgtName == targetName) {
                        projName = tgtName;
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
            } else {
                // Full project mode: collect ALL deps referenced by ANY target.
                // Meson may discover deps during configure that are conditionally
                // skipped and not actually used by any target.
                std::unordered_set<std::string> allDeps;
                for (const auto& t : targetsJson) {
                    if (t.contains("dependencies") && t["dependencies"].is_array()) {
                        for (const auto& dep : t["dependencies"]) {
                            allDeps.insert(dep.get<std::string>());
                        }
                    }
                }
                if (!allDeps.empty()) {
                    targetDependencyNames.assign(allDeps.begin(), allDeps.end());
                }
                // If allDeps is empty (no targets with deps found), keep
                // targetDependencyNames empty = include all intro-dependencies.
            }
        } else if (hasTarget) {
            std::cerr << "Error: Failed to open file: " << targetsPath.string() << std::endl;
            return 1;
        }
        // If intro-targets.json doesn't exist and no --target, include all deps
        // (legacy behavior for build dirs without targets file).
    }

    SBOMBuilder sbom(projName, projVersion);
    PkgConfigWrapper pkgConf;

    // Populate metadata.tools.components with mesonsbom identity using embedded version
    // mesonsbom added first to guarantee it appears first in tools.components array
    sbom.addTool({
        .name = "mesonsbom",
        .version = MESONSBOM_VERSION,
        .description = "CycloneDX SBOM generator from Meson build metadata",
        .supplierName = "Tim Teulings",
        .homepageUrl = "https://github.com/Framstag/mesonsbom",
        .vcsUrl = "git+https://github.com/Framstag/mesonsbom.git",
        .licenses = {"GPL-3.0-or-later"}
    });

    // Add the Meson build system as the second tool entry in tools.components.
    // Version comes from meson-info.json (read earlier) for the analyzed project.
    if (!mesonVersion.empty()) {
        sbom.addTool({
            .name = "meson",
            .version = mesonVersion,
            .description = "Meson is a project to create the best possible "
                           "next-generation build system",
            .supplierName = "The Meson Development Team",
            .homepageUrl = "https://mesonbuild.com",
            .vcsUrl = "git+https://github.com/mesonbuild/meson.git",
            .licenses = {"Apache-2.0"}
        });
    }

    // Add the Meson backend build tool (e.g., ninja) as the third tool entry.
    // Detection from intro-buildoptions.json (read earlier).
    // Version defaults to "unknown" since the build directory does not contain
    // the actual installed backend version.
    if (!backendName.empty() && backendName != "none") {
        ToolInfo backendInfo;
        backendInfo.name = backendName;
        backendInfo.version = "unknown";

        if (backendName == "ninja") {
            backendInfo.description = "Ninja is a small build system with a focus on speed";
            backendInfo.supplierName = "The Ninja Project";
            backendInfo.homepageUrl = "https://ninja-build.org";
            backendInfo.vcsUrl = "git+https://github.com/ninja-build/ninja.git";
            backendInfo.licenses = {"Apache-2.0"};
        } else if (backendName.rfind("vs", 0) == 0) {
            backendInfo.description = "Microsoft Visual Studio build tools";
            backendInfo.homepageUrl = "https://visualstudio.microsoft.com/";
            backendInfo.supplierName = "Microsoft Corporation";
        } else if (backendName == "xcode") {
            backendInfo.description = "Apple Xcode build system";
            backendInfo.homepageUrl = "https://developer.apple.com/xcode/";
            backendInfo.supplierName = "Apple Inc.";
        }

        sbom.addTool(backendInfo);
    }

    // 3.2: Extract license array from intro-projectinfo.json and pass to setLicenses
    std::vector<std::string> projectLicenses;
    if (projectInfo.contains("license") && projectInfo["license"].is_array()) {
        for (const auto& lic : projectInfo["license"]) {
            projectLicenses.push_back(lic.get<std::string>());
        }
    }
    sbom.setLicenses(projectLicenses);

    // =================================================================
    // Data structures for two-phase dependency resolution.
    // =================================================================
    struct RawDependency {
        std::string name;
        std::string version;
        std::string description;
        std::string pcFilePath;
        std::string type;
        bool fromPkgConfig = false;
    };

    struct ResolvedDep {
        std::string osName;
        std::string osVersion;
        std::string osPm;
        std::string pkgCfgName;
        std::string pkgCfgVersion;
        std::string pcFilePath;
        std::string description;
        bool hasOs = false;
    };

    // =================================================================
    // Phase 1: Collect — BFS walk dep graph, store RawDependency list
    // =================================================================
    std::vector<RawDependency> rawDeps;

    // We also need to record parent→child relationships for edge reconstruction.
    struct EdgeRecord {
        std::string parentName;
        std::string childName;
    };
    std::vector<EdgeRecord> rawEdges;

    struct Node {
        std::string name;
        std::vector<std::string> path;
    };
    std::deque<Node> queue;

    // Direct dependencies of main project
    auto [mainFound, directDeps] = loadDependencies(buildDir, "", pkgConf, true);
    if (mainFound && directDeps.is_array()) {
        for (const auto& dep : directDeps) {
            std::string depType = dep.value("type", "");
            if (depType == "optional") continue;

            std::string depName = dep.value("name", "");
            std::string depVersion = dep.value("version", "0.0.0");
            std::string depDescription = dep.value("description", "");

            if (!targetDependencyNames.empty()) {
                bool matchesTarget = false;
                for (const auto& tgtDep : targetDependencyNames) {
                    if (tgtDep == depName) { matchesTarget = true; break; }
                }
                if (!matchesTarget) continue;
            }

            if (!depName.empty()) {
                RawDependency rd;
                rd.name = depName;
                rd.version = depVersion;
                rd.description = depDescription;
                rd.type = depType;
                if (depType == "pkgconfig") {
                    bool found = false;
                    rd.pcFilePath = pkgConf.getPackageFilename(depName, found);
                    rd.fromPkgConfig = found;
                }
                rawDeps.push_back(rd);
                rawEdges.push_back({projName, depName});
                queue.push_back({depName, {projName, depName}});
            }
        }
    }

    // Recursively collect transitive dependencies
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
                    // Circularity detection
                    bool isCircular = false;
                    for (const auto& ancestor : current.path) {
                        if (ancestor == depName) {
                            std::cerr << "Warning: Circular dependency detected: " << depName
                                      << " is an ancestor of itself in the current path." << std::endl;
                            isCircular = true;
                            break;
                        }
                    }
                    if (isCircular) continue;

                    RawDependency rd;
                    rd.name = depName;
                    rd.version = depVersion;
                    rd.description = depDescription;
                    rd.type = depType;
                    if (depType == "pkgconfig") {
                        bool pcFound = false;
                        rd.pcFilePath = pkgConf.getPackageFilename(depName, pcFound);
                        rd.fromPkgConfig = pcFound;
                    }
                    rawDeps.push_back(rd);
                    rawEdges.push_back({current.name, depName});

                    std::vector<std::string> nextPath = current.path;
                    nextPath.push_back(depName);
                    queue.push_back({depName, nextPath});
                }
            }
        }
    }

    // =================================================================
    // Phase 2: Resolve OS packages for pkg-config deps
    // =================================================================
    OsPackageResolver osResolver;
    PackageManager activePm = osResolver.activePackageManager();
    std::string pmName = OsPackageResolver::packageManagerName(activePm);
    OsRelease osRelease = osResolver.osRelease();

    std::vector<ResolvedDep> resolvedDeps;
    for (const auto& rd : rawDeps) {
        ResolvedDep res;
        res.pkgCfgName = rd.name;
        res.pkgCfgVersion = rd.version;
        res.pcFilePath = rd.pcFilePath;
        res.description = rd.description;

        if (rd.fromPkgConfig && !rd.pcFilePath.empty() && activePm != PackageManager::None) {
            auto osPkg = osResolver.resolve(rd.pcFilePath);
            if (osPkg.has_value()) {
                res.osName = osPkg->name;
                res.osVersion = osPkg->version;
                res.osPm = osPkg->packageManager;
                res.hasOs = true;
            }
        }
        resolvedDeps.push_back(res);
    }

    // =================================================================
    // Phase 3: Deduplicate by OS package name
    // =================================================================
    std::vector<ResolvedDep> finalDeps;
    std::unordered_map<std::string, size_t> osToIndex;
    std::unordered_map<std::string, std::string> nameToComponentRef;
    std::unordered_set<std::string> seenPkgCfgNames; // dedup non-OS deps

    for (auto& res : resolvedDeps) {
        if (res.hasOs) {
            auto it = osToIndex.find(res.osName);
            if (it != osToIndex.end()) {
                nameToComponentRef[res.pkgCfgName] = finalDeps[it->second].osName;
                continue;
            }
            osToIndex[res.osName] = finalDeps.size();
            nameToComponentRef[res.pkgCfgName] = res.osName;
            finalDeps.push_back(res);
        } else {
            if (seenPkgCfgNames.count(res.pkgCfgName)) {
                nameToComponentRef[res.pkgCfgName] = res.pkgCfgName;
                continue;
            }
            seenPkgCfgNames.insert(res.pkgCfgName);
            nameToComponentRef[res.pkgCfgName] = res.pkgCfgName;
            finalDeps.push_back(res);
        }
    }

    // =================================================================
    // Phase 4: Build SBOM components with OS identity, evidence, properties
    // =================================================================
    for (const auto& dep : finalDeps) {
        std::string qualifier;
        if (dep.hasOs && !dep.osPm.empty()) {
            qualifier = "os=" + dep.osPm;
        }

        std::string supplierName;
        if (dep.hasOs && !osRelease.name.empty()) {
            supplierName = osRelease.name;
        }

        std::string compName = dep.hasOs ? dep.osName : dep.pkgCfgName;
        std::string compVersion = dep.hasOs ? dep.osVersion : dep.pkgCfgVersion;

        if (!sbom.addComponent(compName, compVersion, "library", dep.description, {}, supplierName, qualifier)) {
            continue;
        }

        if (dep.hasOs) {
            sbom.addEvidenceIdentity(compName, "purl",
                "pkg:generic/" + dep.osName + "@" + dep.osVersion + (qualifier.empty() ? "" : "?" + qualifier),
                1.0, "manifest-analysis",
                "os:package=" + dep.osPm + "|os:name=" + dep.osName + "|os:version=" + dep.osVersion);

            sbom.addEvidenceIdentity(compName, "name", dep.pkgCfgName,
                1.0, "manifest-analysis",
                "pkg-config:" + dep.pkgCfgName + "|pkg-config-version=" + dep.pkgCfgVersion);

            if (!dep.pcFilePath.empty()) {
                sbom.addEvidenceOccurrence(compName, dep.pcFilePath);
            }

            sbom.addProperties(compName, {
                {"discovery:method", "pkg-config"},
                {"pkg-config:name", dep.pkgCfgName},
                {"pkg-config:version", dep.pkgCfgVersion},
                {"os:distribution", osRelease.id},
                {"os:package", dep.osName},
                {"os:package-version", dep.osVersion}
            });
        }
    }

    // =================================================================
    // Replay dependency edges, mapping through nameToComponentRef
    // =================================================================
    for (const auto& edge : rawEdges) {
        std::string fromRef = nameToComponentRef.count(edge.parentName) ? nameToComponentRef[edge.parentName] : edge.parentName;
        std::string toRef = nameToComponentRef.count(edge.childName) ? nameToComponentRef[edge.childName] : edge.childName;
        if (fromRef != toRef) {
            sbom.addDependency(fromRef, toRef);
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