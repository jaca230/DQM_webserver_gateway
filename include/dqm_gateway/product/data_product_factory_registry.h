#pragma once

#include "dqm_gateway/model/product.h"
#include "dqm_gateway/registry/factory_registry.h"

#include <functional>
#include <memory>
#include <string>

namespace dqm_gateway {

class DataProduct;
class DataProductRegistry;
class LimitConfig;
class StoreConfig;

struct DataProductDependencies {
    const StoreConfig& stores;
    const LimitConfig& limits;
    std::function<void(const Product&)> product_observer;
};

class DataProductFactoryRegistry final
    : private FactoryRegistry<
          std::function<std::unique_ptr<DataProduct>(const DataProductDependencies&)>> {
public:
    using Factory = std::function<std::unique_ptr<DataProduct>(const DataProductDependencies&)>;
    using Base = FactoryRegistry<Factory>;

    DataProductFactoryRegistry() : Base("data product") {}

    using Base::add;
    void create_all(
        const DataProductDependencies& dependencies,
        DataProductRegistry& products) const;

};

}  // namespace dqm_gateway
