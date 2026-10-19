#include "dqm_gateway/product/data_product_factory_registry.h"

#include "dqm_gateway/product/data_product_registry.h"

namespace dqm_gateway {

void DataProductFactoryRegistry::create_all(
    const DataProductDependencies& dependencies,
    DataProductRegistry& products) const {
    for (const auto& type : registration_order()) {
        products.add(factory(type)(dependencies));
    }
}

}  // namespace dqm_gateway
