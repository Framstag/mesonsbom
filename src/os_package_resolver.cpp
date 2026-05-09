#include "os_package_resolver.h"

#include <cstdio>
#include <array>
#include <algorithm>
#include <cctype>

// ---------------------------------------------------------------------------
// Helper: run a shell command and return stdout contents.
// Uses popen() for compatibility. Returns empty string on failure.
// ---------------------------------------------------------------------------
std::string OsPackageResolver::execCommand(const std::string& command) const {
    std::string result;
    std::array<char, 4096> buffer{};

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return result;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }

    // pclose returns the exit status; we ignore it since we only care
    // about whether we got output.
    pclose(pipe);

    // Trim trailing whitespace.
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) {
        result.pop_back();
    }

    return result;
}

// ---------------------------------------------------------------------------
// Constructor: detect package manager and parse /etc/os-release.
// ---------------------------------------------------------------------------
OsPackageResolver::OsPackageResolver()
    : pm_(PackageManager::None)
{
    pm_ = detectPackageManager();
    osRelease_ = parseOsRelease();
}

// ---------------------------------------------------------------------------
// Detect which package manager is available by checking for known binaries
// in $PATH via the 'which' command.
// ---------------------------------------------------------------------------
PackageManager OsPackageResolver::detectPackageManager() {
    // Check in priority order.
    if (!execCommand("which pacman 2>/dev/null").empty()) {
        return PackageManager::Pacman;
    }
    if (!execCommand("which dpkg-query 2>/dev/null").empty()) {
        return PackageManager::Dpkg;
    }
    if (!execCommand("which rpm 2>/dev/null").empty()) {
        return PackageManager::Rpm;
    }
    if (!execCommand("which apk 2>/dev/null").empty()) {
        return PackageManager::Apk;
    }
    return PackageManager::None;
}

// ---------------------------------------------------------------------------
// Map enum to human-readable string.
// ---------------------------------------------------------------------------
std::string OsPackageResolver::packageManagerName(PackageManager pm) {
    switch (pm) {
        case PackageManager::Pacman: return "pacman";
        case PackageManager::Dpkg:   return "dpkg";
        case PackageManager::Rpm:    return "rpm";
        case PackageManager::Apk:    return "apk";
        default:                     return "";
    }
}

// ---------------------------------------------------------------------------
// Entry point: check cache, then query the active PM.
// Returns std::nullopt on any failure (file not owned, PM error, etc.).
// ---------------------------------------------------------------------------
std::optional<OsPackage> OsPackageResolver::resolve(const std::string& pcFilePath) {
    // Check cache first.
    auto it = cache_.find(pcFilePath);
    if (it != cache_.end()) {
        return it->second;
    }

    std::optional<OsPackage> result;

    switch (pm_) {
        case PackageManager::Pacman:
            result = queryPacman(pcFilePath);
            break;
        case PackageManager::Dpkg:
            result = queryDpkg(pcFilePath);
            break;
        case PackageManager::Rpm:
            result = queryRpm(pcFilePath);
            break;
        case PackageManager::Apk:
            result = queryApk(pcFilePath);
            break;
        default:
            break;
    }

    // Cache the result (nullopt means "not found, don't retry").
    cache_[pcFilePath] = result;
    return result;
}

// ---------------------------------------------------------------------------
// Parsing helpers for PM output.
// Each returns a trimmed line or empty string.
// ---------------------------------------------------------------------------

// Split on first space, return first token.
static std::string firstToken(const std::string& s) {
    auto pos = s.find_first_of(" \t");
    if (pos == std::string::npos) {
        return s;
    }
    return s.substr(0, pos);
}

// Split on first space, return second token (rest after first space).
static std::string secondToken(const std::string& s) {
    auto pos = s.find_first_of(" \t");
    if (pos == std::string::npos) {
        return {};
    }
    // Skip consecutive spaces.
    auto start = s.find_first_not_of(" \t", pos);
    if (start == std::string::npos) {
        return {};
    }
    return s.substr(start);
}

// ---------------------------------------------------------------------------
// PM-specific query implementations.
// ---------------------------------------------------------------------------
std::optional<OsPackage> OsPackageResolver::queryPacman(const std::string& filePath) const {
    // --quiet only returns the package name, not the version.
    // First call: get the package name.
    std::string nameCmd = "pacman -Qo --quiet " + filePath + " 2>/dev/null";
    std::string name = execCommand(nameCmd);
    if (name.empty()) {
        return std::nullopt;
    }

    // Second call: get the package version.
    // pacman -Q <name> → "<name> <version>"
    std::string verCmd = "pacman -Q " + name + " 2>/dev/null";
    std::string fullLine = execCommand(verCmd);
    std::string version;
    if (!fullLine.empty()) {
        // Parse "<name> <version>" — skip the leading name part.
        version = secondToken(fullLine);
    }

    return OsPackage{name, version, "pacman"};
}

// dpkg -S <path> output format:
//   "curl: /usr/lib/pkgconfig/libcurl.pc"
// Better: dpkg-query -S <path>  →  "curl: /path"
std::optional<OsPackage> OsPackageResolver::queryDpkg(const std::string& filePath) const {
    std::string cmd = "dpkg-query -S " + filePath + " 2>/dev/null";
    std::string output = execCommand(cmd);
    if (output.empty()) {
        return std::nullopt;
    }

    // Output: "<pkg>: <path>" — extract package name before the colon.
    auto colonPos = output.find(':');
    if (colonPos == std::string::npos || colonPos == 0) {
        return std::nullopt;
    }

    std::string name = output.substr(0, colonPos);
    // dpkg-query doesn't easily show version in -S output,
    // so we need a second call: dpkg-query -W -f='${Version}' <pkg>
    std::string verCmd = "dpkg-query -W -f='${Version}' " + name + " 2>/dev/null";
    std::string version = execCommand(verCmd);

    return OsPackage{name, version, "dpkg"};
}

// rpm -qf <path> output format:
//   "curl-7.88.1-1.fc38.x86_64"
std::optional<OsPackage> OsPackageResolver::queryRpm(const std::string& filePath) const {
    std::string cmd = "rpm -qf --queryformat '%{NAME} %{VERSION}-%{RELEASE}' " + filePath + " 2>/dev/null";
    std::string output = execCommand(cmd);
    if (output.empty()) {
        return std::nullopt;
    }

    std::string name = firstToken(output);
    std::string version = secondToken(output);
    if (name.empty()) {
        return std::nullopt;
    }

    return OsPackage{name, version, "rpm"};
}

// apk info --who-owns <path> output format:
//   "/usr/lib/pkgconfig/openssl.pc is owned by openssl-3.1.4-r1"
// Better: apk info -W <path>
std::optional<OsPackage> OsPackageResolver::queryApk(const std::string& filePath) const {
    std::string cmd = "apk info --who-owns " + filePath + " 2>/dev/null";
    std::string output = execCommand(cmd);
    if (output.empty()) {
        return std::nullopt;
    }

    // Output ends with "is owned by <pkg>-<version>"
    auto ownedBy = output.rfind("owned by ");
    if (ownedBy == std::string::npos) {
        return std::nullopt;
    }

    std::string pkgVersion = output.substr(ownedBy + 9); // after "owned by "

    // apk format: "<name>-<version>" — split on last hyphen for version.
    auto lastHyphen = pkgVersion.rfind('-');
    if (lastHyphen == std::string::npos) {
        return std::nullopt;
    }

    std::string name = pkgVersion.substr(0, lastHyphen);
    std::string version = pkgVersion.substr(lastHyphen + 1);

    if (name.empty()) {
        return std::nullopt;
    }

    return OsPackage{name, version, "apk"};
}