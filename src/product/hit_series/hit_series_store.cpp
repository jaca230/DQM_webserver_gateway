#include "dqm_gateway/product/hit_series/hit_series_store.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace dqm_gateway {

HitSeriesStore::HitSeriesStore(
    std::size_t maximum_bytes,
    std::size_t maximum_series,
    std::size_t maximum_hits_per_series)
    : maximum_bytes_(maximum_bytes),
      maximum_series_(maximum_series),
      maximum_hits_per_series_(maximum_hits_per_series) {
    if (maximum_bytes == 0 || maximum_series == 0 || maximum_hits_per_series == 0) {
        throw std::invalid_argument("Hit-series store limits must be positive");
    }
}

std::string HitSeriesStore::key(const HitSeriesSnapshot& series) {
    return series.detector + ':' + std::to_string(series.module) + ':' +
        std::to_string(series.submodule) + ':' + std::to_string(series.channel);
}

std::size_t HitSeriesStore::storage_bytes(const HitSeriesSnapshot& series) {
    return series.detector.size() + series.epoch.size() +
        sizeof(double) * (series.times_ns.size() + series.energies.size()) +
        sizeof(HitSeriesSnapshot);
}

void HitSeriesStore::validate(
    const HitSeriesSnapshot& series,
    std::size_t maximum_hits_per_series) {
    if (series.detector.empty() || series.detector.size() > 64 || series.epoch.empty()) {
        throw std::invalid_argument("Hit-series detector and epoch are required");
    }
    if (series.energies.empty() || series.energies.size() > maximum_hits_per_series ||
        series.times_ns.size() != series.energies.size()) {
        throw std::invalid_argument("Hit-series times and energies must have equal valid lengths");
    }
    const auto finite = [](double value) { return std::isfinite(value); };
    if (!std::all_of(series.times_ns.begin(), series.times_ns.end(), finite) ||
        !std::all_of(series.energies.begin(), series.energies.end(), finite)) {
        throw std::invalid_argument("Hit-series values must be finite");
    }
}

void HitSeriesStore::update(std::vector<HitSeriesSnapshot> series) {
    for (const auto& item : series) validate(item, maximum_hits_per_series_);

    std::unique_lock lock(mutex_);
    auto projected = series_;
    auto projected_bytes = bytes_;
    for (auto& item : series) {
        const auto item_key = key(item);
        const auto existing = projected.find(item_key);
        if (existing != projected.end()) {
            projected_bytes -= storage_bytes(existing->second);
            if (existing->second.epoch == item.epoch) {
                auto& previous = existing->second;
                previous.times_ns.insert(
                    previous.times_ns.end(), item.times_ns.begin(), item.times_ns.end());
                previous.energies.insert(
                    previous.energies.end(), item.energies.begin(), item.energies.end());
                if (previous.energies.size() > maximum_hits_per_series_) {
                    const auto remove_count =
                        previous.energies.size() - maximum_hits_per_series_;
                    previous.times_ns.erase(
                        previous.times_ns.begin(), previous.times_ns.begin() + remove_count);
                    previous.energies.erase(
                        previous.energies.begin(), previous.energies.begin() + remove_count);
                }
                previous.sequence = item.sequence;
                previous.generated_at_unix_ns = item.generated_at_unix_ns;
                projected_bytes += storage_bytes(previous);
                continue;
            }
        }
        projected_bytes += storage_bytes(item);
        projected.insert_or_assign(item_key, std::move(item));
    }
    if (projected.size() > maximum_series_ || projected_bytes > maximum_bytes_) {
        throw std::length_error("Hit-series store capacity exceeded");
    }
    series_ = std::move(projected);
    bytes_ = projected_bytes;
}

nlohmann::json HitSeriesStore::view(
    const std::string& detector,
    std::optional<int> module,
    std::optional<int> submodule,
    std::optional<int> channel,
    std::size_t maximum_hits) const {
    if (maximum_hits == 0 || maximum_hits > maximum_hits_per_series_) {
        throw std::invalid_argument("Invalid maximum hit count");
    }
    std::shared_lock lock(mutex_);
    nlohmann::json result{
        {"schema", "dqm-hit-series-view-v1"},
        {"detector", detector},
        {"generated-at-unix-ns", 0},
        {"hit-series", nlohmann::json::array()},
    };
    for (const auto& [item_key, item] : series_) {
        (void)item_key;
        if (item.detector != detector ||
            (module && item.module != *module) ||
            (submodule && item.submodule != *submodule) ||
            (channel && item.channel != *channel)) continue;
        const auto stride = std::max<std::size_t>(
            1, (item.energies.size() + maximum_hits - 1) / maximum_hits);
        nlohmann::json times = nlohmann::json::array();
        nlohmann::json energies = nlohmann::json::array();
        for (std::size_t index = 0; index < item.energies.size(); index += stride) {
            times.push_back(item.times_ns[index]);
            energies.push_back(item.energies[index]);
        }
        result["hit-series"].push_back({
            {"module", item.module},
            {"submodule", item.submodule},
            {"channel", item.channel},
            {"sequence", item.sequence},
            {"generated-at-unix-ns", item.generated_at_unix_ns},
            {"epoch", item.epoch},
            {"times-ns", std::move(times)},
            {"energies", std::move(energies)},
        });
        result["generated-at-unix-ns"] = std::max(
            result["generated-at-unix-ns"].get<std::int64_t>(),
            item.generated_at_unix_ns);
    }
    return result;
}

nlohmann::json HitSeriesStore::catalog() const {
    std::shared_lock lock(mutex_);
    nlohmann::json result = nlohmann::json::array();
    for (const auto& [item_key, item] : series_) {
        (void)item_key;
        result.push_back({
            {"detector", item.detector}, {"module", item.module},
            {"submodule", item.submodule}, {"channel", item.channel},
            {"hits", item.energies.size()}, {"epoch", item.epoch},
            {"sequence", item.sequence},
            {"generated-at-unix-ns", item.generated_at_unix_ns},
        });
    }
    return result;
}

nlohmann::json HitSeriesStore::catalog_json() const {
    return catalog();
}

std::size_t HitSeriesStore::stored_items() const {
    std::shared_lock lock(mutex_);
    return series_.size();
}

std::size_t HitSeriesStore::stored_bytes() const {
    std::shared_lock lock(mutex_);
    return bytes_;
}

}  // namespace dqm_gateway
