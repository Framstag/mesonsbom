#pragma once

#include <string>

// ---------------------------------------------------------------------------
// OS release information parsed from /etc/os-release (or equivalent).
// Fields map to the standard os-release(5) keys:
//   ID=           short machine-readable identifier (e.g., "arch", "debian")
//   NAME=         human-readable distribution name (e.g., "Arch Linux")
//   VERSION_ID=   optional version string (e.g., "11", "38")
// ---------------------------------------------------------------------------
struct OsRelease {
    std::string id;         // "arch", "debian", "ubuntu", "fedora", ...
    std::string name;       // "Arch Linux", "Debian GNU/Linux", ...
    std::string versionId;  // may be empty
};

// Parse /etc/os-release and return the fields. On failure (file missing,
// unreadable, or parse error) returns an OsRelease with all empty fields.
OsRelease parseOsRelease(const std::string& path = "/etc/os-release");
