#pragma once

#include <cstdint>

namespace dqm_gateway {

class PluginRegistry;
inline constexpr std::uint32_t kPluginAbiVersion = 1;

}  // namespace dqm_gateway

#if defined(_WIN32)
#define DQM_GATEWAY_PLUGIN_EXPORT __declspec(dllexport)
#else
#define DQM_GATEWAY_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#define DQM_GATEWAY_PLUGIN(REGISTRATION_FUNCTION)                                      \
    extern "C" DQM_GATEWAY_PLUGIN_EXPORT std::uint32_t                               \
    dqm_gateway_plugin_abi_version() {                                                 \
        return ::dqm_gateway::kPluginAbiVersion;                                       \
    }                                                                                  \
    extern "C" DQM_GATEWAY_PLUGIN_EXPORT void dqm_gateway_register_plugin(           \
        ::dqm_gateway::PluginRegistry& registry) {                                     \
        REGISTRATION_FUNCTION(registry);                                               \
    }
