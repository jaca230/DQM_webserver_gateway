#include "dqm_gateway/plugin/plugin_entry.h"
#include "dqm_gateway/plugin/plugin_registry.h"
#include "dqm_gateway/product/histogram/histogram_product.h"

namespace {

void register_plugin(dqm_gateway::PluginRegistry& registry) {
    registry.add_data_product<dqm_gateway::HistogramProduct>("histograms");
}

}  // namespace

DQM_GATEWAY_PLUGIN(register_plugin)
