#include "dqm_gateway/api/route/system/register_system_routes.h"

#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/api/route/system/catalog_route.h"
#include "dqm_gateway/api/route/system/health_route.h"
#include "dqm_gateway/api/route/system/metrics_route.h"

#include <memory>

namespace dqm_gateway {

void register_system_routes(
    ApiRouteRegistry& routes,
    DataSourceRegistry& sources,
    DataProductRegistry& products,
    GatewayMetrics& metrics) {
    routes.add(std::make_unique<HealthRoute>(sources, products));
    routes.add(std::make_unique<CatalogRoute>(products));
    routes.add(std::make_unique<MetricsRoute>(metrics, products));
}

}  // namespace dqm_gateway
