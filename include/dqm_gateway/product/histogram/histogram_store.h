#pragma once

#include "dqm_gateway/model/stores/data_store.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

struct HistogramSnapshot {
    std::string name;
    std::string title;
    std::string units;
    std::string definition;
    std::string epoch;
    std::uint64_t sequence{0};
    std::int64_t generated_at_unix_ns{0};
    std::vector<double> edges;
    std::vector<double> counts;
    std::vector<double> sumw2;
};

class HistogramStore final : public DataStore {
public:
    HistogramStore(
        std::size_t maximum_view_bytes,
        std::size_t maximum_histograms,
        std::size_t maximum_baselines,
        std::size_t maximum_baseline_bytes);

    void update(HistogramSnapshot histogram);
    void update(std::vector<HistogramSnapshot> histograms);
    bool create_baseline(const std::string& histogram_name, const std::string& baseline_name);
    nlohmann::json view(
        const std::string& histogram_name,
        const std::optional<std::string>& baseline_name,
        std::size_t maximum_bins) const;
    nlohmann::json catalog() const;
    [[nodiscard]] std::string_view name() const noexcept override { return "histograms"; }
    [[nodiscard]] nlohmann::json catalog_json() const override;
    [[nodiscard]] std::size_t stored_items() const override;
    [[nodiscard]] std::size_t stored_bytes() const override;

private:
    struct BaselineKey {
        std::string histogram;
        std::string baseline;

        bool operator==(const BaselineKey&) const = default;
    };

    struct BaselineKeyHash {
        std::size_t operator()(const BaselineKey& key) const noexcept;
    };

    static void validate(const HistogramSnapshot& histogram);
    static std::size_t storage_bytes(const HistogramSnapshot& histogram);

    const std::size_t maximum_view_bytes_;
    const std::size_t maximum_histograms_;
    const std::size_t maximum_baselines_;
    const std::size_t maximum_baseline_bytes_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, HistogramSnapshot> histograms_;
    std::unordered_map<BaselineKey, HistogramSnapshot, BaselineKeyHash> baselines_;
    std::size_t view_bytes_{0};
    std::size_t baseline_bytes_{0};
};

}  // namespace dqm_gateway
