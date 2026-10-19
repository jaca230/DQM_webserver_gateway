#include "dqm_gateway/config/sections/store_config.h"

#include "dqm_gateway/config/config_value.h"

#include <stdexcept>

namespace dqm_gateway {

void StoreConfig::parse(const nlohmann::json& value) {
    config_value::validate_keys(
        value,
        {"maximum-total-bytes", "maximum-products", "maximum-view-bytes",
         "maximum-histograms", "maximum-waveforms", "maximum-baselines",
         "maximum-baseline-bytes", "maximum-hit-series"},
        name());
    config_value::read_optional(value, "maximum-total-bytes", maximum_total_bytes);
    config_value::read_optional(value, "maximum-products", maximum_products);
    config_value::read_optional(value, "maximum-view-bytes", maximum_view_bytes);
    config_value::read_optional(value, "maximum-histograms", maximum_histograms);
    config_value::read_optional(value, "maximum-hit-series", maximum_hit_series);
    config_value::read_optional(value, "maximum-waveforms", maximum_waveforms);
    config_value::read_optional(value, "maximum-baselines", maximum_baselines);
    config_value::read_optional(value, "maximum-baseline-bytes", maximum_baseline_bytes);
    if (maximum_total_bytes == 0 || maximum_products == 0 || maximum_view_bytes == 0 ||
        maximum_histograms == 0 || maximum_waveforms == 0 || maximum_hit_series == 0 ||
        maximum_baselines == 0 || maximum_baseline_bytes == 0) {
        throw std::invalid_argument("Store limits must be nonzero");
    }
}

}  // namespace dqm_gateway
