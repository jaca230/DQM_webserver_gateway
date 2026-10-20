#include "dqm_gateway/product/atar/atar_product.h"
#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/config/sections/store_config.h"
#include "dqm_gateway/product/atar/routes/atar_route.h"
#include "dqm_gateway/product/data_product_factory_registry.h"

#include <memory>
namespace dqm_gateway {
AtarProduct::AtarProduct(const DataProductDependencies& dependencies)
    : store_(dependencies.stores.maximum_view_bytes), decoder_(store_) {}
bool AtarProduct::accepts(const Product& product) const { return decoder_.accepts(product); }
bool AtarProduct::ingest(const Product& product) { decoder_.decode(product); return true; }
void AtarProduct::register_routes(ApiRouteRegistry& routes) {
    routes.add(std::make_unique<AtarRoute>(store_));
}
}  // namespace dqm_gateway
