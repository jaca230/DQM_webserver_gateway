#pragma once

#include <string_view>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class ConfigSection {
public:
    virtual ~ConfigSection() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    virtual void parse(const nlohmann::json& value) = 0;
};

}  // namespace dqm_gateway
