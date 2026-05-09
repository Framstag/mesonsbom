#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include "os_release.h"

// ---------------------------------------------------------------------------
// Identifies a system package manager available on the build host.
// Detection order: pacman → dpkg → rpm → apk → None.
// ---------------------------------------------------------------------------
enum class PackageManager {
    None,
    Pacman,
    Dpkg,
    Rpm,
    Apk
};

// ---------------------------------------------------------------------------
// Result of an OS package ownership query.
// ---------------------------------------------------------------------------
struct OsPackage {
    std::string name;          // e.g., "curl"
    std::string version;       // e.g., "8.20.0-5"
    std::string packageManager; // e.g., "pacman"
};

// ---------------------------------------------------------------------------
// Resolves which OS package owns a given file by querying the system package
// manager. Results are cached by file path to avoid redundant subprocess
// calls.
// ---------------------------------------------------------------------------
class OsPackageResolver {
public:
    OsPackageResolver();

    // Detect the system package manager (scans $PATH for known binaries).
    PackageManager detectPackageManager();

    // Query which OS package owns the given file path.
    // Returns std::nullopt if the file is not owned by any known package,
    // or if the package manager cannot be queried.
    std::optional<OsPackage> resolve(const std::string& pcFilePath);

    // Returns the detected package manager (call detectPackageManager() first).
    PackageManager activePackageManager() const { return pm_; }

    // Returns the parsed OS release info (may be all-empty on failure).
    const OsRelease& osRelease() const { return osRelease_; }

    // Returns a human-readable name for a package manager enum value.
    static std::string packageManagerName(PackageManager pm);

private:
    PackageManager pm_;
    OsRelease osRelease_;
    std::unordered_map<std::string, std::optional<OsPackage>> cache_;

    // Run a subprocess and capture stdout. Returns empty string on failure.
    std::string execCommand(const std::string& command) const;

    // PM-specific query builders.
    std::optional<OsPackage> queryPacman(const std::string& filePath) const;
    std::optional<OsPackage> queryDpkg(const std::string& filePath) const;
    std::optional<OsPackage> queryRpm(const std::string& filePath) const;
    std::optional<OsPackage> queryApk(const std::string& filePath) const;
};