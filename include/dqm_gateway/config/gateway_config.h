#pragma once

#include "dqm_gateway/config/config_section_registry.h"
#include "dqm_gateway/config/sections/data_source_config.h"
#include "dqm_gateway/config/sections/limit_config.h"
#include "dqm_gateway/config/sections/server_config.h"
#include "dqm_gateway/config/sections/store_config.h"

#include <filesystem>

namespace dqm_gateway {

class GatewayConfig {
public:
    static GatewayConfig from_file(const std::filesystem::path& path);

    const ServerConfig& server() const { return sections_.get<ServerConfig>(); }
    const DataSourceConfig& sources() const { return sections_.get<DataSourceConfig>(); }
    const StoreConfig& store() const { return sections_.get<StoreConfig>(); }
    const LimitConfig& limits() const { return sections_.get<LimitConfig>(); }

private:
    GatewayConfig();
    ConfigSectionRegistry sections_;
};

}  // namespace dqm_gateway
