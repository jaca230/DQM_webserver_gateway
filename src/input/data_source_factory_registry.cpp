#include "dqm_gateway/input/data_source_factory_registry.h"

#include "dqm_gateway/input/data_source_registry.h"

namespace dqm_gateway {

void DataSourceFactoryRegistry::create_all(
    const DataSourceConfig& config,
    const DataSourceDependencies& dependencies,
    DataSourceRegistry& sources) const {
    for (const auto& definition : config.definitions()) {
        sources.add(factory(definition.type)(definition, dependencies));
    }
}

}  // namespace dqm_gateway
