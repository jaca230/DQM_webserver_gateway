#include "dqm_gateway/product/waveform/waveform_product.h"

#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/config/sections/limit_config.h"
#include "dqm_gateway/config/sections/store_config.h"
#include "dqm_gateway/product/data_product_factory_registry.h"
#include "dqm_gateway/product/waveform/routes/waveform_route.h"

#include <memory>

namespace dqm_gateway {

WaveformProduct::WaveformProduct(const DataProductDependencies& dependencies)
    : store_(
          dependencies.stores.maximum_view_bytes,
          dependencies.stores.maximum_waveforms,
          dependencies.limits.maximum_waveform_samples),
      decoder_(store_) {}

bool WaveformProduct::accepts(const Product& product) const {
    return decoder_.accepts(product);
}

bool WaveformProduct::ingest(const Product& product) {
    decoder_.decode(product);
    return true;
}

void WaveformProduct::register_routes(ApiRouteRegistry& routes) {
    routes.add(std::make_unique<WaveformRoute>(
        store_, store_.maximum_samples_per_waveform()));
}

}  // namespace dqm_gateway
