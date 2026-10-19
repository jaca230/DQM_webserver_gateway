#pragma once

namespace dqm_gateway {

class ApiContext;

class ApiRoute {
public:
    virtual ~ApiRoute() = default;
    virtual void register_route(ApiContext& context) = 0;
};

}  // namespace dqm_gateway
