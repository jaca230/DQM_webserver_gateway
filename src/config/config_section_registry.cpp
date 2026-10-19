#include "dqm_gateway/config/config_section_registry.h"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <utility>

namespace dqm_gateway {

void ConfigSectionRegistry::add_section(std::unique_ptr<ConfigSection> section) {
    if (!section) throw std::invalid_argument("Cannot register a null config section");
    const std::string name(section->name());
    if (name.empty()) throw std::invalid_argument("Config section name cannot be empty");
    if (!sections_.emplace(name, std::move(section)).second) {
        throw std::invalid_argument("Duplicate configuration section: " + name);
    }
}

void ConfigSectionRegistry::parse(const nlohmann::json& root) {
    if (!root.is_object()) throw std::invalid_argument("root must be an object");
    for (const auto& [name, value] : root.items()) {
        const auto section = sections_.find(name);
        if (section == sections_.end()) {
            throw std::invalid_argument("Unknown configuration section '" + name + "'");
        }
        section->second->parse(value);
    }
}

}  // namespace dqm_gateway
