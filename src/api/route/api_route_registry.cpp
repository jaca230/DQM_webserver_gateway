#include "dqm_gateway/api/route/api_route_registry.h"

#include "dqm_gateway/api/context/api_context.h"

#include <stdexcept>
#include <utility>

namespace dqm_gateway {

void ApiRouteRegistry::add(std::unique_ptr<ApiRoute> route) {
    if (registered_) throw std::logic_error("Cannot add a route after registration");
    if (!route) throw std::invalid_argument("Cannot register a null API route");
    routes_.push_back(std::move(route));
}

void ApiRouteRegistry::register_routes() {
    if (registered_) return;
    for (const auto& route : routes_) route->register_route(context_);
    registered_ = true;
}

}  // namespace dqm_gateway
