#include "dqm_gateway/plugin/plugin_entry.h"
#include "dqm_gateway/plugin/plugin_registry.h"
#include "dqm_gateway/product/hit_series/hit_series_product.h"

namespace {

void register_plugin(dqm_gateway::PluginRegistry& registry) {
    registry.add_data_product<dqm_gateway::HitSeriesProduct>("hit-series");
}

}  // namespace

DQM_GATEWAY_PLUGIN(register_plugin)
