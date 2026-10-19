#pragma once

#include "dqm_gateway/model/stores/data_store.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace dqm_gateway {

struct WaveformSnapshot {
    std::string detector;
    std::string epoch;
    int module{0};
    int submodule{0};
    int channel{0};
    double sample_period_ns{0.0};
    std::uint64_t sequence{0};
    std::int64_t generated_at_unix_ns{0};
    std::vector<double> samples;
    std::vector<double> times_ns;
};

class WaveformStore final : public DataStore {
public:
    WaveformStore(
        std::size_t maximum_bytes,
        std::size_t maximum_waveforms,
        std::size_t maximum_samples_per_waveform);

    void update(std::vector<WaveformSnapshot> waveforms);
    nlohmann::json view(
        const std::string& detector,
        std::optional<int> module,
        std::optional<int> submodule,
        std::optional<int> channel,
        std::size_t maximum_samples) const;
    nlohmann::json catalog() const;
    [[nodiscard]] std::string_view name() const noexcept override { return "waveforms"; }
    [[nodiscard]] nlohmann::json catalog_json() const override;
    [[nodiscard]] std::size_t stored_items() const override;
    [[nodiscard]] std::size_t stored_bytes() const override;
    [[nodiscard]] std::size_t maximum_samples_per_waveform() const noexcept {
        return maximum_samples_per_waveform_;
    }

private:
    static std::string key(const WaveformSnapshot& waveform);
    static std::size_t storage_bytes(const WaveformSnapshot& waveform);
    static void validate(
        const WaveformSnapshot& waveform,
        std::size_t maximum_samples_per_waveform);

    std::size_t maximum_bytes_;
    std::size_t maximum_waveforms_;
    std::size_t maximum_samples_per_waveform_;
    std::size_t bytes_{0};
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, WaveformSnapshot> waveforms_;
};

}  // namespace dqm_gateway
