#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace dqm_gateway::config_value {

inline void validate_keys(
    const nlohmann::json& value,
    std::initializer_list<std::string_view> allowed,
    std::string_view location) {
    if (!value.is_object()) {
        throw std::invalid_argument(std::string(location) + " must be an object");
    }
    for (const auto& [key, ignored] : value.items()) {
        (void)ignored;
        bool found = false;
        for (const auto candidate : allowed) found = found || key == candidate;
        if (!found) {
            throw std::invalid_argument(
                "Unknown configuration key '" + key + "' in " + std::string(location));
        }
    }
}

template <typename T>
void read_optional(const nlohmann::json& value, const char* key, T& target) {
    if (const auto found = value.find(key); found != value.end()) target = found->get<T>();
}

}  // namespace dqm_gateway::config_value
