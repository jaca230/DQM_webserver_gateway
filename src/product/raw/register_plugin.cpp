#include "dqm_gateway/plugin/plugin_entry.h"
#include "dqm_gateway/plugin/plugin_registry.h"
#include "dqm_gateway/product/raw/raw_product.h"

namespace {

void register_plugin(dqm_gateway::PluginRegistry& registry) {
    registry.add_data_product<dqm_gateway::RawProduct>("products");
}

}  // namespace

DQM_GATEWAY_PLUGIN(register_plugin)
