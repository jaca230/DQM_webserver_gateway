#pragma once

#include "dqm_gateway/api/route/api_route.h"

namespace dqm_gateway {

class DataProductRegistry;
class GatewayMetrics;

class MetricsRoute final : public ApiRoute {
public:
    MetricsRoute(GatewayMetrics& metrics, DataProductRegistry& products)
        : metrics_(metrics), products_(products) {}

    void register_route(ApiContext& context) override;

private:
    GatewayMetrics& metrics_;
    DataProductRegistry& products_;
};

}  // namespace dqm_gateway
