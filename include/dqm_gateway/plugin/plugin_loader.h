#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dqm_gateway {

class PluginRegistry;

class PluginLoader final {
public:
    PluginLoader() = default;
    ~PluginLoader();

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    void load_directory(
        const std::filesystem::path& directory,
        PluginRegistry& registry);

    [[nodiscard]] const std::vector<std::string>& loaded_plugins() const noexcept {
        return loaded_plugins_;
    }

private:
    std::vector<void*> handles_;
    std::vector<std::string> loaded_plugins_;
};

}  // namespace dqm_gateway
