#include "dqm_gateway/product/hit_series/hit_series_product.h"

#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/config/sections/limit_config.h"
#include "dqm_gateway/config/sections/store_config.h"
#include "dqm_gateway/product/data_product_factory_registry.h"
#include "dqm_gateway/product/hit_series/routes/hit_series_route.h"

#include <memory>

namespace dqm_gateway {

HitSeriesProduct::HitSeriesProduct(const DataProductDependencies& dependencies)
    : store_(
          dependencies.stores.maximum_view_bytes,
          dependencies.stores.maximum_hit_series,
          dependencies.limits.maximum_hits_per_series),
      decoder_(store_) {}

bool HitSeriesProduct::accepts(const Product& product) const {
    return decoder_.accepts(product);
}

bool HitSeriesProduct::ingest(const Product& product) {
    decoder_.decode(product);
    return true;
}

void HitSeriesProduct::register_routes(ApiRouteRegistry& routes) {
    routes.add(std::make_unique<HitSeriesRoute>(
        store_, store_.maximum_hits_per_series()));
}

}  // namespace dqm_gateway
