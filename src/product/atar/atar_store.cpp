#include "dqm_gateway/product/atar/atar_store.h"

#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace dqm_gateway {
AtarStore::AtarStore(std::size_t maximum_bytes) : maximum_bytes_(maximum_bytes) {
    if (maximum_bytes == 0) throw std::invalid_argument("ATAR store limit must be positive");
}
void AtarStore::update(nlohmann::json product) {
    const auto view = product.value("view", "");
    if (view.empty()) throw std::invalid_argument("ATAR view product has no view");
    const auto bytes = product.dump().size();
    if (bytes > maximum_bytes_) throw std::length_error("ATAR product exceeds store limit");
    std::unique_lock lock(mutex_);
    const auto existing = views_.find(view);
    const auto replaced_bytes = existing == views_.end() ? 0 : existing->second.dump().size();
    if (bytes_ - replaced_bytes + bytes > maximum_bytes_)
        throw std::length_error("ATAR views exceed store limit");
    bytes_ = bytes_ - replaced_bytes + bytes;
    views_[view] = std::move(product);
}
nlohmann::json AtarStore::view(const std::string& name) const {
    std::shared_lock lock(mutex_);
    const auto found = views_.find(name);
    return found == views_.end() ? nlohmann::json::object() : found->second;
}
nlohmann::json AtarStore::catalog_json() const {
    std::shared_lock lock(mutex_);
    auto products = nlohmann::json::array();
    for (const auto& [name, product] : views_)
        products.push_back({{"view", name},
            {"sequence", product.value("sequence", std::uint64_t{0})}});
    return {{"available", !views_.empty()}, {"schema", "dqm-atar-view-v1"},
        {"views", std::move(products)}};
}
std::size_t AtarStore::stored_items() const { std::shared_lock lock(mutex_); return views_.size(); }
std::size_t AtarStore::stored_bytes() const { std::shared_lock lock(mutex_); return bytes_; }
}  // namespace dqm_gateway
