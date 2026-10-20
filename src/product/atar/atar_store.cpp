#include "dqm_gateway/product/atar/atar_store.h"

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dqm_gateway {
AtarStore::AtarStore(std::size_t maximum_bytes) : maximum_bytes_(maximum_bytes) {
    if (maximum_bytes == 0) throw std::invalid_argument("ATAR store limit must be positive");
}
void AtarStore::update(nlohmann::json product) {
    const auto bytes = product.dump().size();
    if (bytes > maximum_bytes_) throw std::length_error("ATAR product exceeds store limit");
    std::unique_lock lock(mutex_);
    product_ = std::move(product);
    bytes_ = bytes;
}
nlohmann::json AtarStore::view(const std::string& name) const {
    static const std::vector<std::string> allowed{
        "occupancy", "hits-per-event", "baseline", "noise", "event",
        "persistence", "charge-vs-amplitude", "amplitude-by-channel"};
    if (std::find(allowed.begin(), allowed.end(), name) == allowed.end())
        throw std::invalid_argument("unknown ATAR view");
    std::shared_lock lock(mutex_);
    if (product_.empty() || !product_.contains(name)) return nlohmann::json::object();
    return {{"schema", "dqm-atar-view-v1"}, {"view", name},
        {"epoch", product_.value("epoch", "")},
        {"sequence", product_.value("sequence", std::uint64_t{0})},
        {"generated-at-unix-ns", product_.value("generated-at-unix-ns", std::int64_t{0})},
        {"geometry", product_.value("geometry", nlohmann::json::object())},
        {"data", product_.at(name)}};
}
nlohmann::json AtarStore::catalog_json() const {
    std::shared_lock lock(mutex_);
    return {{"available", !product_.empty()}, {"schema", "dqm-atar-v1"},
        {"sequence", product_.value("sequence", std::uint64_t{0})}};
}
std::size_t AtarStore::stored_items() const { std::shared_lock lock(mutex_); return product_.empty() ? 0 : 1; }
std::size_t AtarStore::stored_bytes() const { std::shared_lock lock(mutex_); return bytes_; }
}  // namespace dqm_gateway
