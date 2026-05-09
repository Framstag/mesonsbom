#include "os_release.h"

#include <fstream>
#include <cstring>
#include <cerrno>

// ---------------------------------------------------------------------------
// Parse a single "KEY=VALUE" or "KEY=\"VALUE\"" line.
// Strips surrounding quotes (both " and ') and leading/trailing whitespace.
// Returns the value part (empty if line is malformed or not a KEY= pair).
// ---------------------------------------------------------------------------
static std::string parseLine(const std::string& line) {
    // Skip comments and empty lines.
    if (line.empty() || line[0] == '#') {
        return {};
    }

    auto eqPos = line.find('=');
    if (eqPos == std::string::npos) {
        return {};
    }

    std::string value = line.substr(eqPos + 1);

    // Trim leading whitespace.
    auto firstNonSpace = value.find_first_not_of(" \t");
    if (firstNonSpace != std::string::npos) {
        value = value.substr(firstNonSpace);
    }

    // Strip surrounding quotes.
    if (value.size() >= 2 && (value[0] == '"' || value[0] == '\'')) {
        char quote = value[0];
        if (value.back() == quote) {
            value = value.substr(1, value.size() - 2);
        }
    }

    // Trim trailing whitespace.
    auto lastNonSpace = value.find_last_not_of(" \t");
    if (lastNonSpace != std::string::npos) {
        value = value.substr(0, lastNonSpace + 1);
    }

    return value;
}

// ---------------------------------------------------------------------------
// Parse a stream of os-release(5) formatted lines into an OsRelease struct.
// Only ID, NAME, and VERSION_ID are extracted; other keys are ignored.
// ---------------------------------------------------------------------------
static OsRelease parseStream(std::istream& stream) {
    OsRelease release;
    std::string line;

    while (std::getline(stream, line)) {
        if (line.size() >= 3 && line.substr(0, 3) == "ID=") {
            release.id = parseLine(line);
        } else if (line.size() >= 5 && line.substr(0, 5) == "NAME=") {
            release.name = parseLine(line);
        } else if (line.size() >= 11 && line.substr(0, 11) == "VERSION_ID=") {
            release.versionId = parseLine(line);
        }
    }

    return release;
}

// ---------------------------------------------------------------------------
// Public entry point: open the os-release file and parse it.
// Returns an empty (all-fields-empty) struct on any failure.
// ---------------------------------------------------------------------------
OsRelease parseOsRelease(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {}; // all fields empty — silent fallback
    }

    return parseStream(file);
}
