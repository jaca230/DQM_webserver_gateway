#include "dqm_gateway/product/raw/raw_product.h"

#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/config/sections/store_config.h"
#include "dqm_gateway/product/data_product_factory_registry.h"
#include "dqm_gateway/product/raw/routes/product_json_route.h"
#include "dqm_gateway/product/raw/routes/product_metadata_route.h"
#include "dqm_gateway/product/raw/routes/product_payload_route.h"

#include <memory>

namespace dqm_gateway {

RawProduct::RawProduct(const DataProductDependencies& dependencies)
    : store_(
          dependencies.stores.maximum_total_bytes,
          dependencies.stores.maximum_products) {
    store_.set_update_callback(dependencies.product_observer);
}

bool RawProduct::ingest(const Product& product) {
    return store_.put(product);
}

void RawProduct::register_routes(ApiRouteRegistry& routes) {
    routes.add(std::make_unique<ProductMetadataRoute>(store_));
    routes.add(std::make_unique<ProductJsonRoute>(store_));
    routes.add(std::make_unique<ProductPayloadRoute>(store_));
}

}  // namespace dqm_gateway
