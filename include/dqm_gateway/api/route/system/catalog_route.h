#pragma once

#include "dqm_gateway/api/route/api_route.h"

namespace dqm_gateway {

class DataProductRegistry;

class CatalogRoute final : public ApiRoute {
public:
    explicit CatalogRoute(DataProductRegistry& products) : products_(products) {}
    void register_route(ApiContext& context) override;

private:
    DataProductRegistry& products_;
};

}  // namespace dqm_gateway
