// src/pkg_config_wrapper.h

#ifndef PKG_CONFIG_WRAPPER_H
#define PKG_CONFIG_WRAPPER_H

#include <string>
#include <vector>
#include <libpkgconf/libpkgconf.h>

// Result struct for getPackageInfo
struct PackageInfo {
    std::string name;
    std::string version;
    std::string description;
};

// Forward declaration for RAII encapsulation
struct pkgconf_client_;

class PkgConfigWrapper {
public:
    PkgConfigWrapper();
    ~PkgConfigWrapper();
    
    // Retrieve transitive dependencies of a package name.
    // Returns a vector of dependency names.
    // Sets 'found' to true if the package was found via pkg-config.
    std::vector<std::string> getDependencies(const std::string& pkgName, bool& found);
    
    // Retrieve the description of a package (from the .pc file's Description: field).
    // Returns the description string, or empty string if not found.
    // Sets 'found' to true if the package was found via pkg-config.
    std::string getPackageDescription(const std::string& pkgName, bool& found);
    
    // Retrieve all available info (name, version, description) for a package.
    // Returns a PackageInfo struct with available fields filled in.
    // Sets 'found' to true if the package was found via pkg-config.
    PackageInfo getPackageInfo(const std::string& pkgName, bool& found);
    
private:
    pkgconf_client_t* ctx;
};

#endif // PKG_CONFIG_WRAPPER_H