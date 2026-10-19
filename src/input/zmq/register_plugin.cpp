#include "dqm_gateway/input/zmq/zmq_publisher_source.h"
#include "dqm_gateway/plugin/plugin_entry.h"
#include "dqm_gateway/plugin/plugin_registry.h"

namespace {

void register_plugin(dqm_gateway::PluginRegistry& registry) {
    registry.add_data_source<dqm_gateway::ZmqPublisherSource>("zmq-publisher");
}

}  // namespace

DQM_GATEWAY_PLUGIN(register_plugin)
