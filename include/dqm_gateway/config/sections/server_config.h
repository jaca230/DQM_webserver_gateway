#pragma once

#include "dqm_gateway/config/config_section.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dqm_gateway {

class ServerConfig final : public ConfigSection {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "server"; }
    void parse(const nlohmann::json& value) override;

    std::string address{"127.0.0.1"};
    std::uint16_t port{8080};
    std::size_t threads{2};
    std::vector<std::string> allowed_origins;
};

}  // namespace dqm_gateway
