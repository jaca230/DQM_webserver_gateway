#include "dqm_gateway/plugin/plugin_loader.h"

#include "dqm_gateway/plugin/plugin_entry.h"
#include "dqm_gateway/plugin/plugin_registry.h"

#include <algorithm>
#include <dlfcn.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace dqm_gateway {
namespace {

using VersionFunction = std::uint32_t (*)();
using RegistrationFunction = void (*)(PluginRegistry&);

template <typename Function>
Function symbol(void* handle, const char* name, const std::filesystem::path& plugin) {
    dlerror();
    auto* address = dlsym(handle, name);
    if (const auto* error = dlerror()) {
        throw std::runtime_error(
            "Plugin '" + plugin.string() + "' does not export " + name + ": " + error);
    }
    return reinterpret_cast<Function>(address);
}

}  // namespace

PluginLoader::~PluginLoader() {
    for (auto iterator = handles_.rbegin(); iterator != handles_.rend(); ++iterator) {
        dlclose(*iterator);
    }
}

void PluginLoader::load_directory(
    const std::filesystem::path& directory,
    PluginRegistry& registry) {
    if (!std::filesystem::is_directory(directory)) {
        throw std::runtime_error("Plugin directory does not exist: " + directory.string());
    }

    std::vector<std::filesystem::path> plugins;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        const auto& path = entry.path();
        if (entry.is_regular_file() && path.extension() == ".so" &&
            path.filename().string().starts_with("dqm_gateway_")) {
            plugins.push_back(path);
        }
    }
    std::sort(plugins.begin(), plugins.end());
    if (plugins.empty()) {
        throw std::runtime_error("No DQM gateway plugins found in: " + directory.string());
    }

    for (const auto& plugin : plugins) {
        void* handle = dlopen(plugin.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            throw std::runtime_error(
                "Could not load plugin '" + plugin.string() + "': " + dlerror());
        }
        try {
            const auto version = symbol<VersionFunction>(
                handle, "dqm_gateway_plugin_abi_version", plugin);
            if (version() != kPluginAbiVersion) {
                throw std::runtime_error("Plugin ABI version mismatch: " + plugin.string());
            }
            const auto registration = symbol<RegistrationFunction>(
                handle, "dqm_gateway_register_plugin", plugin);
            registration(registry);
            handles_.push_back(handle);
            loaded_plugins_.push_back(plugin.filename().string());
        } catch (...) {
            dlclose(handle);
            throw;
        }
    }
}

}  // namespace dqm_gateway
