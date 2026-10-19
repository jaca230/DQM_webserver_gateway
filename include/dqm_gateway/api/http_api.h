#pragma once

namespace dqm_gateway {

class ApiRouteRegistry;

class HttpApi final {
public:
    explicit HttpApi(ApiRouteRegistry& routes) : routes_(routes) {}

    void register_routes();

private:
    ApiRouteRegistry& routes_;
};

}  // namespace dqm_gateway
