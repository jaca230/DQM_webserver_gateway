#pragma once

#include "dqm_gateway/api/route/data/data_route.h"

namespace dqm_gateway {

class WaveformStore;

class WaveformRoute final : public DataRoute {
public:
    WaveformRoute(WaveformStore& store, std::size_t maximum_samples);
    void register_route(ApiContext& context) override;

private:
    WaveformStore& waveforms_;
    std::size_t maximum_samples_;
};

}  // namespace dqm_gateway
