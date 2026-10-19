#pragma once

#include "dqm_gateway/api/route/data/data_route.h"

namespace dqm_gateway {

class HitSeriesStore;

class HitSeriesRoute final : public DataRoute {
public:
    HitSeriesRoute(HitSeriesStore& store, std::size_t maximum_hits);
    void register_route(ApiContext& context) override;

private:
    HitSeriesStore& hit_series_;
    std::size_t maximum_hits_;
};

}  // namespace dqm_gateway
