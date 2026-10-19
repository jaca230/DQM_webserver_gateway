#include "dqm_gateway/product/histogram/histogram_store.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dqm_gateway {

HistogramStore::HistogramStore(
    std::size_t maximum_view_bytes,
    std::size_t maximum_histograms,
    std::size_t maximum_baselines,
    std::size_t maximum_baseline_bytes)
    : maximum_view_bytes_(maximum_view_bytes),
      maximum_histograms_(maximum_histograms),
      maximum_baselines_(maximum_baselines),
      maximum_baseline_bytes_(maximum_baseline_bytes) {
    if (maximum_view_bytes == 0 || maximum_histograms == 0 ||
        maximum_baselines == 0 || maximum_baseline_bytes == 0) {
        throw std::invalid_argument("Histogram store limits must be positive");
    }
}

std::size_t HistogramStore::BaselineKeyHash::operator()(const BaselineKey& key) const noexcept {
    const auto first = std::hash<std::string>{}(key.histogram);
    const auto second = std::hash<std::string>{}(key.baseline);
    return first ^ (second + 0x9e3779b9U + (first << 6U) + (first >> 2U));
}

void HistogramStore::validate(const HistogramSnapshot& histogram) {
    if (histogram.name.empty() || histogram.definition.empty() || histogram.epoch.empty()) {
        throw std::invalid_argument("Histogram name, definition, and epoch are required");
    }
    if (histogram.counts.empty() || histogram.edges.size() != histogram.counts.size() + 1) {
        throw std::invalid_argument("Histogram edges must contain one more value than counts");
    }
    if (!histogram.sumw2.empty() && histogram.sumw2.size() != histogram.counts.size()) {
        throw std::invalid_argument("Histogram sumw2 must match the count array");
    }
    if (!std::is_sorted(histogram.edges.begin(), histogram.edges.end()) ||
        std::adjacent_find(
            histogram.edges.begin(), histogram.edges.end(),
            [](double left, double right) { return left >= right; }) != histogram.edges.end()) {
        throw std::invalid_argument("Histogram edges must be strictly increasing");
    }
}

std::size_t HistogramStore::storage_bytes(const HistogramSnapshot& histogram) {
    return histogram.name.size() + histogram.title.size() + histogram.units.size() +
        histogram.definition.size() + histogram.epoch.size() +
        sizeof(double) *
            (histogram.edges.size() + histogram.counts.size() + histogram.sumw2.size());
}

void HistogramStore::update(HistogramSnapshot histogram) {
    std::vector<HistogramSnapshot> histograms;
    histograms.push_back(std::move(histogram));
    update(std::move(histograms));
}

void HistogramStore::update(std::vector<HistogramSnapshot> histograms) {
    for (const auto& histogram : histograms) validate(histogram);
    std::unique_lock lock(mutex_);
    std::size_t projected_bytes = view_bytes_;
    std::size_t projected_count = histograms_.size();
    std::unordered_map<std::string, std::size_t> staged_bytes;
    std::vector<HistogramSnapshot> accepted;
    accepted.reserve(histograms.size());

    for (auto& histogram : histograms) {
        const auto existing = histograms_.find(histogram.name);
        if (existing != histograms_.end() && existing->second.sequence > histogram.sequence &&
            existing->second.epoch == histogram.epoch) {
            continue;
        }
        const auto staged = staged_bytes.find(histogram.name);
        if (staged != staged_bytes.end()) {
            throw std::invalid_argument("Histogram product contains a duplicate name");
        }
        const auto old_bytes = existing == histograms_.end()
            ? 0
            : storage_bytes(existing->second);
        if (existing == histograms_.end()) ++projected_count;
        const auto new_bytes = storage_bytes(histogram);
        projected_bytes = projected_bytes - old_bytes + new_bytes;
        staged_bytes.insert_or_assign(histogram.name, new_bytes);
        accepted.push_back(std::move(histogram));
    }
    if (projected_count > maximum_histograms_) {
        throw std::length_error("Histogram count limit reached");
    }
    if (projected_bytes > maximum_view_bytes_) {
        throw std::length_error("Histogram view byte limit reached");
    }
    view_bytes_ = projected_bytes;
    for (auto& histogram : accepted) {
        const auto name = histogram.name;
        histograms_.insert_or_assign(name, std::move(histogram));
    }
}

bool HistogramStore::create_baseline(
    const std::string& histogram_name,
    const std::string& baseline_name) {
    if (baseline_name.empty() || baseline_name.size() > 128) {
        throw std::invalid_argument("Baseline name must contain 1 to 128 characters");
    }
    std::unique_lock lock(mutex_);
    const auto found = histograms_.find(histogram_name);
    if (found == histograms_.end()) return false;
    const BaselineKey key{histogram_name, baseline_name};
    const auto existing = baselines_.find(key);
    const auto old_bytes = existing == baselines_.end() ? 0 : storage_bytes(existing->second);
    const auto new_bytes = storage_bytes(found->second);
    if (existing == baselines_.end() && baselines_.size() >= maximum_baselines_) {
        throw std::length_error("Histogram baseline count limit reached");
    }
    if (baseline_bytes_ - old_bytes + new_bytes > maximum_baseline_bytes_) {
        throw std::length_error("Histogram baseline byte limit reached");
    }
    baseline_bytes_ = baseline_bytes_ - old_bytes + new_bytes;
    baselines_.insert_or_assign(key, found->second);
    return true;
}

nlohmann::json HistogramStore::view(
    const std::string& histogram_name,
    const std::optional<std::string>& baseline_name,
    std::size_t maximum_bins) const {
    if (maximum_bins == 0) throw std::invalid_argument("maximum_bins must be positive");

    HistogramSnapshot current;
    std::optional<HistogramSnapshot> baseline;
    {
        std::shared_lock lock(mutex_);
        const auto found = histograms_.find(histogram_name);
        if (found == histograms_.end()) throw std::out_of_range("Histogram not found");
        current = found->second;
        if (baseline_name) {
            const auto base = baselines_.find(BaselineKey{histogram_name, *baseline_name});
            if (base == baselines_.end()) throw std::out_of_range("Histogram baseline not found");
            baseline = base->second;
        }
    }

    auto counts = current.counts;
    auto sumw2 = current.sumw2;
    if (baseline) {
        if (baseline->definition != current.definition || baseline->epoch != current.epoch ||
            baseline->edges != current.edges || baseline->counts.size() != current.counts.size()) {
            throw std::logic_error("Histogram baseline belongs to a different definition or epoch");
        }
        for (std::size_t index = 0; index < counts.size(); ++index) {
            counts[index] -= baseline->counts[index];
        }
        if (!sumw2.empty() && baseline->sumw2.size() == sumw2.size()) {
            for (std::size_t index = 0; index < sumw2.size(); ++index) {
                sumw2[index] = std::max(0.0, sumw2[index] - baseline->sumw2[index]);
            }
        }
    }

    const auto factor = std::max<std::size_t>(
        1, (counts.size() + maximum_bins - 1) / maximum_bins);
    std::vector<double> output_edges;
    std::vector<double> output_counts;
    std::vector<double> output_sumw2;
    output_edges.reserve((counts.size() + factor - 1) / factor + 1);
    output_counts.reserve((counts.size() + factor - 1) / factor);
    if (!sumw2.empty()) output_sumw2.reserve(output_counts.capacity());

    output_edges.push_back(current.edges.front());
    for (std::size_t begin = 0; begin < counts.size(); begin += factor) {
        const auto end = std::min(counts.size(), begin + factor);
        double count = 0.0;
        double variance = 0.0;
        for (auto index = begin; index < end; ++index) {
            count += counts[index];
            if (!sumw2.empty()) variance += sumw2[index];
        }
        output_counts.push_back(count);
        if (!sumw2.empty()) output_sumw2.push_back(variance);
        output_edges.push_back(current.edges[end]);
    }

    nlohmann::json result{
        {"name", current.name},
        {"title", current.title},
        {"units", current.units},
        {"definition", current.definition},
        {"epoch", current.epoch},
        {"sequence", current.sequence},
        {"generated-at-unix-ns", current.generated_at_unix_ns},
        {"baseline", baseline_name ? nlohmann::json(*baseline_name) : nlohmann::json(nullptr)},
        {"binning-factor", factor},
        {"edges", output_edges},
        {"counts", output_counts},
    };
    if (!output_sumw2.empty()) result["sumw2"] = std::move(output_sumw2);
    return result;
}

nlohmann::json HistogramStore::catalog() const {
    std::shared_lock lock(mutex_);
    nlohmann::json result = nlohmann::json::array();
    for (const auto& [name, histogram] : histograms_) {
        result.push_back({
            {"name", name},
            {"title", histogram.title},
            {"units", histogram.units},
            {"definition", histogram.definition},
            {"epoch", histogram.epoch},
            {"sequence", histogram.sequence},
            {"generated-at-unix-ns", histogram.generated_at_unix_ns},
            {"bins", histogram.counts.size()},
        });
    }
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.at("name").template get<std::string>() <
               right.at("name").template get<std::string>();
    });
    return result;
}

nlohmann::json HistogramStore::catalog_json() const {
    return catalog();
}

std::size_t HistogramStore::stored_items() const {
    std::shared_lock lock(mutex_);
    return histograms_.size();
}

std::size_t HistogramStore::stored_bytes() const {
    std::shared_lock lock(mutex_);
    return view_bytes_;
}

}  // namespace dqm_gateway
