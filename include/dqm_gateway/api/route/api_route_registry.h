#pragma once

#include "dqm_gateway/api/route/api_route.h"

#include <memory>
#include <vector>

namespace dqm_gateway {

class ApiContext;

class ApiRouteRegistry final {
public:
    explicit ApiRouteRegistry(ApiContext& context) : context_(context) {}
    void add(std::unique_ptr<ApiRoute> route);
    void register_routes();

private:
    ApiContext& context_;
    std::vector<std::unique_ptr<ApiRoute>> routes_;
    bool registered_{false};
};

}  // namespace dqm_gateway
