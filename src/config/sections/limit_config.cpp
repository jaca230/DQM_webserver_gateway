#include "dqm_gateway/config/sections/limit_config.h"

#include "dqm_gateway/config/config_value.h"

#include <stdexcept>

namespace dqm_gateway {

void LimitConfig::parse(const nlohmann::json& value) {
    config_value::validate_keys(
        value,
        {"requests-per-second", "request-burst", "maximum-response-bytes",
         "maximum-histogram-bins", "maximum-waveform-samples",
         "maximum-hits-per-series", "websocket-maximum-rate-hz"},
        name());
    config_value::read_optional(value, "requests-per-second", requests_per_second);
    config_value::read_optional(value, "request-burst", request_burst);
    config_value::read_optional(value, "maximum-response-bytes", maximum_response_bytes);
    config_value::read_optional(value, "maximum-histogram-bins", maximum_histogram_bins);
    config_value::read_optional(value, "maximum-hits-per-series", maximum_hits_per_series);
    config_value::read_optional(value, "maximum-waveform-samples", maximum_waveform_samples);
    config_value::read_optional(value, "websocket-maximum-rate-hz", websocket_maximum_rate_hz);
    if (requests_per_second <= 0.0 || request_burst == 0 || maximum_response_bytes == 0 ||
        maximum_histogram_bins == 0 || maximum_hits_per_series == 0 ||
        maximum_waveform_samples == 0 || websocket_maximum_rate_hz <= 0.0) {
        throw std::invalid_argument("Request and response limits must be positive");
    }
}

}  // namespace dqm_gateway
