#include "dqm_gateway/config/sections/server_config.h"

#include "dqm_gateway/config/config_value.h"

#include <stdexcept>

namespace dqm_gateway {

void ServerConfig::parse(const nlohmann::json& value) {
    config_value::validate_keys(
        value, {"address", "port", "threads", "allowed-origins"}, name());
    config_value::read_optional(value, "address", address);
    config_value::read_optional(value, "port", port);
    config_value::read_optional(value, "threads", threads);
    config_value::read_optional(value, "allowed-origins", allowed_origins);
    if (address.empty() || port == 0 || threads == 0) {
        throw std::invalid_argument("Server address, port, and thread count must be nonzero");
    }
}

}  // namespace dqm_gateway
