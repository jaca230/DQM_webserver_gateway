#pragma once

#include "dqm_gateway/config/config_section.h"

#include <cstddef>
#include <string_view>

namespace dqm_gateway {

class LimitConfig final : public ConfigSection {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "limits"; }
    void parse(const nlohmann::json& value) override;

    double requests_per_second{20.0};
    std::size_t request_burst{40};
    std::size_t maximum_response_bytes{8U * 1024U * 1024U};
    std::size_t maximum_histogram_bins{4096};
    std::size_t maximum_hits_per_series{8192};
    std::size_t maximum_waveform_samples{8192};
    double websocket_maximum_rate_hz{10.0};
};

}  // namespace dqm_gateway
