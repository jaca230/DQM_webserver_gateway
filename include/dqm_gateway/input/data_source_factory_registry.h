#pragma once

#include "dqm_gateway/config/sections/data_source_config.h"
#include "dqm_gateway/model/product.h"
#include "dqm_gateway/registry/factory_registry.h"

#include <functional>
#include <memory>
#include <string>

namespace dqm_gateway {

class DataSource;
class DataSourceRegistry;
class GatewayMetrics;

struct DataSourceDependencies {
    std::function<void(Product)> product_handler;
    GatewayMetrics& metrics;
};

class DataSourceFactoryRegistry final
    : private FactoryRegistry<std::function<std::unique_ptr<DataSource>(
          const DataSourceDefinition&,
          const DataSourceDependencies&)>> {
public:
    using Factory = std::function<std::unique_ptr<DataSource>(
        const DataSourceDefinition&,
        const DataSourceDependencies&)>;
    using Base = FactoryRegistry<Factory>;

    DataSourceFactoryRegistry() : Base("data source") {}

    using Base::add;
    void create_all(
        const DataSourceConfig& config,
        const DataSourceDependencies& dependencies,
        DataSourceRegistry& sources) const;

};

}  // namespace dqm_gateway
