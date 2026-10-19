#pragma once

#include "dqm_gateway/api/route/api_route.h"

namespace dqm_gateway {

class DataProductRegistry;
class DataSourceRegistry;

class HealthRoute final : public ApiRoute {
public:
    HealthRoute(DataSourceRegistry& sources, DataProductRegistry& products)
        : sources_(sources), products_(products) {}

    void register_route(ApiContext& context) override;

private:
    DataSourceRegistry& sources_;
    DataProductRegistry& products_;
};

}  // namespace dqm_gateway
