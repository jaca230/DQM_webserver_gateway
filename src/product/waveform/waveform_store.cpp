#include "dqm_gateway/product/waveform/waveform_store.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace dqm_gateway {

WaveformStore::WaveformStore(
    std::size_t maximum_bytes,
    std::size_t maximum_waveforms,
    std::size_t maximum_samples_per_waveform)
    : maximum_bytes_(maximum_bytes),
      maximum_waveforms_(maximum_waveforms),
      maximum_samples_per_waveform_(maximum_samples_per_waveform) {
    if (maximum_bytes == 0 || maximum_waveforms == 0 || maximum_samples_per_waveform == 0) {
        throw std::invalid_argument("Waveform store limits must be positive");
    }
}

std::string WaveformStore::key(const WaveformSnapshot& waveform) {
    return waveform.detector + ':' + std::to_string(waveform.module) + ':' +
        std::to_string(waveform.submodule) + ':' + std::to_string(waveform.channel);
}

std::size_t WaveformStore::storage_bytes(const WaveformSnapshot& waveform) {
    return waveform.detector.size() + waveform.epoch.size() +
        sizeof(double) * (waveform.samples.size() + waveform.times_ns.size()) +
        sizeof(WaveformSnapshot);
}

void WaveformStore::validate(
    const WaveformSnapshot& waveform,
    std::size_t maximum_samples_per_waveform) {
    if (waveform.detector.empty() || waveform.detector.size() > 64 || waveform.epoch.empty()) {
        throw std::invalid_argument("Waveform detector and epoch are required");
    }
    if (waveform.samples.empty() || waveform.samples.size() > maximum_samples_per_waveform) {
        throw std::invalid_argument("Waveform sample count is outside the configured limit");
    }
    if (!waveform.times_ns.empty() && waveform.times_ns.size() != waveform.samples.size()) {
        throw std::invalid_argument("Waveform times and samples must have equal lengths");
    }
    if (waveform.times_ns.empty() && waveform.sample_period_ns <= 0.0) {
        throw std::invalid_argument("Uniform waveforms require a positive sample period");
    }
    if (!std::all_of(waveform.samples.begin(), waveform.samples.end(), [](double value) {
            return std::isfinite(value);
        })) {
        throw std::invalid_argument("Waveform samples must be finite");
    }
}

void WaveformStore::update(std::vector<WaveformSnapshot> waveforms) {
    for (const auto& waveform : waveforms) validate(waveform, maximum_samples_per_waveform_);

    std::unique_lock lock(mutex_);
    auto projected = waveforms_;
    std::size_t projected_bytes = bytes_;
    for (auto& waveform : waveforms) {
        const auto waveform_key = key(waveform);
        const auto existing = projected.find(waveform_key);
        if (existing != projected.end()) projected_bytes -= storage_bytes(existing->second);
        projected_bytes += storage_bytes(waveform);
        projected.insert_or_assign(waveform_key, std::move(waveform));
    }
    if (projected.size() > maximum_waveforms_ || projected_bytes > maximum_bytes_) {
        throw std::length_error("Waveform store capacity exceeded");
    }
    waveforms_ = std::move(projected);
    bytes_ = projected_bytes;
}

nlohmann::json WaveformStore::view(
    const std::string& detector,
    std::optional<int> module,
    std::optional<int> submodule,
    std::optional<int> channel,
    std::size_t maximum_samples) const {
    if (maximum_samples == 0 || maximum_samples > maximum_samples_per_waveform_) {
        throw std::invalid_argument("Invalid maximum waveform sample count");
    }
    std::shared_lock lock(mutex_);
    nlohmann::json result{{"schema", "dqm-waveform-v1"}, {"detector", detector},
                          {"generated-at-unix-ns", 0},
                          {"waveforms", nlohmann::json::array()}};
    for (const auto& [waveform_key, waveform] : waveforms_) {
        (void)waveform_key;
        if (waveform.detector != detector ||
            (module && waveform.module != *module) ||
            (submodule && waveform.submodule != *submodule) ||
            (channel && waveform.channel != *channel)) continue;
        const auto stride = std::max<std::size_t>(
            1, (waveform.samples.size() + maximum_samples - 1) / maximum_samples);
        nlohmann::json samples = nlohmann::json::array();
        nlohmann::json times = nlohmann::json::array();
        for (std::size_t index = 0; index < waveform.samples.size(); index += stride) {
            samples.push_back(waveform.samples[index]);
            if (!waveform.times_ns.empty()) times.push_back(waveform.times_ns[index]);
        }
        nlohmann::json item{
            {"module", waveform.module},
            {"submodule", waveform.submodule},
            {"channel", waveform.channel},
            {"sample-period-ns", waveform.sample_period_ns * static_cast<double>(stride)},
            {"sequence", waveform.sequence},
            {"generated-at-unix-ns", waveform.generated_at_unix_ns},
            {"epoch", waveform.epoch},
            {"samples", std::move(samples)},
        };
        if (!times.empty()) item["times-ns"] = std::move(times);
        result["waveforms"].push_back(std::move(item));
        result["generated-at-unix-ns"] = std::max(
            result["generated-at-unix-ns"].get<std::int64_t>(),
            waveform.generated_at_unix_ns);
    }
    return result;
}

nlohmann::json WaveformStore::catalog() const {
    std::shared_lock lock(mutex_);
    nlohmann::json result = nlohmann::json::array();
    for (const auto& [waveform_key, waveform] : waveforms_) {
        (void)waveform_key;
        result.push_back({
            {"detector", waveform.detector}, {"module", waveform.module},
            {"submodule", waveform.submodule}, {"channel", waveform.channel},
            {"samples", waveform.samples.size()}, {"epoch", waveform.epoch},
            {"sequence", waveform.sequence},
            {"generated-at-unix-ns", waveform.generated_at_unix_ns},
        });
    }
    return result;
}

nlohmann::json WaveformStore::catalog_json() const {
    return catalog();
}

std::size_t WaveformStore::stored_items() const {
    std::shared_lock lock(mutex_);
    return waveforms_.size();
}

std::size_t WaveformStore::stored_bytes() const {
    std::shared_lock lock(mutex_);
    return bytes_;
}

}  // namespace dqm_gateway
