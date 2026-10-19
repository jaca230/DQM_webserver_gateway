#include "dqm_gateway/product/histogram/histogram_product.h"

#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/config/sections/store_config.h"
#include "dqm_gateway/product/data_product_factory_registry.h"
#include "dqm_gateway/product/histogram/routes/histogram_baseline_route.h"
#include "dqm_gateway/product/histogram/routes/histogram_route.h"

#include <memory>

namespace dqm_gateway {

HistogramProduct::HistogramProduct(const DataProductDependencies& dependencies)
    : store_(
          dependencies.stores.maximum_view_bytes,
          dependencies.stores.maximum_histograms,
          dependencies.stores.maximum_baselines,
          dependencies.stores.maximum_baseline_bytes),
      decoder_(store_) {}

bool HistogramProduct::accepts(const Product& product) const {
    return decoder_.accepts(product);
}

bool HistogramProduct::ingest(const Product& product) {
    decoder_.decode(product);
    return true;
}

void HistogramProduct::register_routes(ApiRouteRegistry& routes) {
    routes.add(std::make_unique<HistogramRoute>(store_));
    routes.add(std::make_unique<HistogramBaselineRoute>(store_));
}

}  // namespace dqm_gateway
