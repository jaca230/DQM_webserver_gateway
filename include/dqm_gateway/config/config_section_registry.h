#pragma once

#include "dqm_gateway/config/config_section.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class ConfigSectionRegistry final {
public:
    template <typename Section, typename... Arguments>
    Section& add(Arguments&&... arguments) {
        static_assert(std::is_base_of_v<ConfigSection, Section>);
        auto section = std::make_unique<Section>(std::forward<Arguments>(arguments)...);
        auto* result = section.get();
        add_section(std::move(section));
        return *result;
    }

    void parse(const nlohmann::json& root);

    template <typename Section>
    [[nodiscard]] const Section& get() const {
        static_assert(std::is_base_of_v<ConfigSection, Section>);
        for (const auto& [name, section] : sections_) {
            (void)name;
            if (const auto* typed = dynamic_cast<const Section*>(section.get())) return *typed;
        }
        throw std::logic_error("Configuration section type is not registered");
    }

private:
    void add_section(std::unique_ptr<ConfigSection> section);
    std::unordered_map<std::string, std::unique_ptr<ConfigSection>> sections_;
};

}  // namespace dqm_gateway
