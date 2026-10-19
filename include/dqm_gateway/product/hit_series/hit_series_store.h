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

struct HitSeriesSnapshot {
    std::string detector;
    std::string epoch;
    int module{0};
    int submodule{0};
    int channel{0};
    std::uint64_t sequence{0};
    std::int64_t generated_at_unix_ns{0};
    std::vector<double> times_ns;
    std::vector<double> energies;
};

class HitSeriesStore final : public DataStore {
public:
    HitSeriesStore(
        std::size_t maximum_bytes,
        std::size_t maximum_series,
        std::size_t maximum_hits_per_series);

    void update(std::vector<HitSeriesSnapshot> series);
    nlohmann::json view(
        const std::string& detector,
        std::optional<int> module,
        std::optional<int> submodule,
        std::optional<int> channel,
        std::size_t maximum_hits) const;
    nlohmann::json catalog() const;
    [[nodiscard]] std::string_view name() const noexcept override { return "hit-series"; }
    [[nodiscard]] nlohmann::json catalog_json() const override;
    [[nodiscard]] std::size_t stored_items() const override;
    [[nodiscard]] std::size_t stored_bytes() const override;
    [[nodiscard]] std::size_t maximum_hits_per_series() const noexcept {
        return maximum_hits_per_series_;
    }

private:
    static std::string key(const HitSeriesSnapshot& series);
    static std::size_t storage_bytes(const HitSeriesSnapshot& series);
    static void validate(
        const HitSeriesSnapshot& series,
        std::size_t maximum_hits_per_series);

    std::size_t maximum_bytes_;
    std::size_t maximum_series_;
    std::size_t maximum_hits_per_series_;
    std::size_t bytes_{0};
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, HitSeriesSnapshot> series_;
};

}  // namespace dqm_gateway
