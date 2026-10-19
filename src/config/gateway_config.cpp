#include "dqm_gateway/config/gateway_config.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dqm_gateway {

GatewayConfig::GatewayConfig() {
    sections_.add<ServerConfig>();
    sections_.add<DataSourceConfig>();
    sections_.add<StoreConfig>();
    sections_.add<LimitConfig>();
}

GatewayConfig GatewayConfig::from_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Could not open configuration: " + path.string());

    nlohmann::json root;
    input >> root;
    GatewayConfig result;
    result.sections_.parse(root);
    return result;
}

}  // namespace dqm_gateway
