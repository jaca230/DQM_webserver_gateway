#include "dqm_gateway/api/http_api.h"

#include "dqm_gateway/api/route/api_route_registry.h"

namespace dqm_gateway {

void HttpApi::register_routes() {
    routes_.register_routes();
}

}  // namespace dqm_gateway
