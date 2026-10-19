#pragma once

#include "dqm_gateway/api/route/data/data_route.h"

namespace dqm_gateway {

class HistogramStore;

class HistogramRoute final : public DataRoute {
public:
    explicit HistogramRoute(HistogramStore& store);
    void register_route(ApiContext& context) override;

private:
    HistogramStore& histograms_;
};

}  // namespace dqm_gateway
