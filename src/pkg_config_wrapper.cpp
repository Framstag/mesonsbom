#include "pkg_config_wrapper.h"
#include <libpkgconf/libpkgconf.h>
#include <iostream>
#include <algorithm>

PkgConfigWrapper::PkgConfigWrapper() {
    pkgconf_cross_personality_t *personality = pkgconf_cross_personality_default();
    ctx = pkgconf_client_new(nullptr, nullptr, personality);
    
    if (ctx) {
        pkgconf_client_dir_list_build(ctx, personality);
    }
}

PkgConfigWrapper::~PkgConfigWrapper() {
    if (ctx) {
        pkgconf_client_free(ctx);
    }
}

std::vector<std::string> PkgConfigWrapper::getDependencies(const std::string& pkgName, bool& found) {
    found = false;
    std::vector<std::string> dependencies;

    if (!ctx) {
        return dependencies;
    }

    // Find the package in the client's current world
    pkgconf_pkg_t* pkg = pkgconf_pkg_find(ctx, pkgName.c_str());
    if (!pkg) {
        return dependencies;
    }

    found = true;

    // pkg->required is a pkgconf_list_t (struct with head, tail, length).
    // We iterate through the linked list of nodes, each node's .data field
    // contains a pointer to the actual pkgconf_dependency_t.
    pkgconf_node_t *node = pkg->required.head;
    while (node != nullptr) {
        // The data pointer in each node points to the dependency object
        pkgconf_dependency_t *dep = static_cast<pkgconf_dependency_t*>(node->data);
        if (dep && dep->package) {
            dependencies.push_back(dep->package);
        }
        node = node->next;
    }

    return dependencies;
}

std::string PkgConfigWrapper::getPackageDescription(const std::string& pkgName, bool& found) {
    found = false;

    if (!ctx) {
        return "";
    }

    pkgconf_pkg_t* pkg = pkgconf_pkg_find(ctx, pkgName.c_str());
    if (!pkg) {
        return "";
    }

    found = true;

    if (pkg->description) {
        return std::string(pkg->description);
    }

    return "";
}

PackageInfo PkgConfigWrapper::getPackageInfo(const std::string& pkgName, bool& found) {
    PackageInfo info;
    found = false;

    if (!ctx) {
        return info;
    }

    pkgconf_pkg_t* pkg = pkgconf_pkg_find(ctx, pkgName.c_str());
    if (!pkg) {
        return info;
    }

    found = true;

    if (pkg->id) {
        info.name = pkg->id;
    }
    if (pkg->version) {
        info.version = pkg->version;
    }
    if (pkg->description) {
        info.description = pkg->description;
    }

    return info;
}

    std::string PkgConfigWrapper::getPackageFilename(const std::string& pkgName, bool& found) {
    found = false;
    if (!ctx) {
        return "";
    }

    pkgconf_pkg_t* pkg = pkgconf_pkg_find(ctx, pkgName.c_str());
    if (!pkg) {
        return "";
    }

    found = true;
    if (pkg->filename) {
        return std::string(pkg->filename);
    }
    return "";
}