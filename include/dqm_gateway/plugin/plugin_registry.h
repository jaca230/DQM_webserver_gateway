#pragma once

#include "dqm_gateway/input/data_source_factory_registry.h"
#include "dqm_gateway/product/data_product_factory_registry.h"

#include <memory>
#include <string>
#include <utility>

namespace dqm_gateway {

class PluginRegistry final {
public:
    PluginRegistry(
        DataProductFactoryRegistry& products,
        DataSourceFactoryRegistry& sources)
        : products_(products), sources_(sources) {}

    template <typename ProductType>
    void add_data_product(std::string type) {
        products_.add(
            std::move(type),
            [](const DataProductDependencies& dependencies) {
                return std::make_unique<ProductType>(dependencies);
            });
    }

    template <typename SourceType>
    void add_data_source(std::string type) {
        sources_.add(
            std::move(type),
            [](const DataSourceDefinition& definition,
               const DataSourceDependencies& dependencies) {
                return std::make_unique<SourceType>(definition, dependencies);
            });
    }

private:
    DataProductFactoryRegistry& products_;
    DataSourceFactoryRegistry& sources_;
};

}  // namespace dqm_gateway
