#pragma once

namespace dqm_gateway {

class ApiRouteRegistry;
class DataProductRegistry;
class DataSourceRegistry;
class GatewayMetrics;

void register_system_routes(
    ApiRouteRegistry& routes,
    DataSourceRegistry& sources,
    DataProductRegistry& products,
    GatewayMetrics& metrics);

}  // namespace dqm_gateway
