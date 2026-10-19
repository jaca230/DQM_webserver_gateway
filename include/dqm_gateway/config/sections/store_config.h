#pragma once

#include "dqm_gateway/config/config_section.h"

#include <cstddef>
#include <string_view>

namespace dqm_gateway {

class StoreConfig final : public ConfigSection {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "store"; }
    void parse(const nlohmann::json& value) override;

    std::size_t maximum_total_bytes{256U * 1024U * 1024U};
    std::size_t maximum_products{1024};
    std::size_t maximum_view_bytes{256U * 1024U * 1024U};
    std::size_t maximum_histograms{4096};
    std::size_t maximum_hit_series{4096};
    std::size_t maximum_waveforms{4096};
    std::size_t maximum_baselines{1024};
    std::size_t maximum_baseline_bytes{64U * 1024U * 1024U};
};

}  // namespace dqm_gateway
