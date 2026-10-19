#pragma once

#include "dqm_gateway/api/route/data/data_route.h"

namespace dqm_gateway {

class ProductStore;

class ProductMetadataRoute final : public DataRoute {
public:
    explicit ProductMetadataRoute(ProductStore& store);
    void register_route(ApiContext& context) override;

private:
    ProductStore& products_;
};

}  // namespace dqm_gateway
