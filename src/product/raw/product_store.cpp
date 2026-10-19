#include "dqm_gateway/product/raw/product_store.h"

#include <algorithm>
#include <limits>
#include <mutex>
#include <utility>

namespace dqm_gateway {

ProductStore::ProductStore(
    std::size_t maximum_total_bytes,
    std::size_t maximum_products)
    : maximum_total_bytes_(maximum_total_bytes),
      maximum_products_(maximum_products) {}

bool ProductStore::put(Product product) {
    if (!product.payload || product.id.empty() || product.size() > maximum_total_bytes_) {
        return false;
    }
    const auto product_id = product.id;

    UpdateCallback callback;
    Product callback_product;
    {
        std::unique_lock lock(mutex_);
        const auto existing = products_.find(product_id);
        if (existing != products_.end()) bytes_ -= existing->second.product.size();
        evict_to_fit(product.size(), product_id);
        if (bytes_ + product.size() > maximum_total_bytes_) return false;

        auto [position, inserted] = products_.insert_or_assign(
            product_id, Entry{std::move(product), ++age_});
        (void)inserted;
        bytes_ += position->second.product.size();
        callback = update_callback_;
        callback_product = position->second.product;
    }
    if (callback) callback(callback_product);
    return true;
}

std::optional<Product> ProductStore::get(const std::string& id) const {
    std::shared_lock lock(mutex_);
    const auto found = products_.find(id);
    if (found == products_.end()) return std::nullopt;
    return found->second.product;
}

std::vector<Product> ProductStore::catalog() const {
    std::shared_lock lock(mutex_);
    std::vector<Product> result;
    result.reserve(products_.size());
    for (const auto& [id, entry] : products_) result.push_back(entry.product);
    std::sort(result.begin(), result.end(), [](const Product& left, const Product& right) {
        return left.id < right.id;
    });
    return result;
}

nlohmann::json ProductStore::catalog_json() const {
    auto result = nlohmann::json::array();
    for (const auto& product : catalog()) result.push_back(product_summary(product));
    return result;
}

void ProductStore::set_update_callback(UpdateCallback callback) {
    std::unique_lock lock(mutex_);
    update_callback_ = std::move(callback);
}

std::size_t ProductStore::bytes() const {
    std::shared_lock lock(mutex_);
    return bytes_;
}

std::size_t ProductStore::size() const {
    std::shared_lock lock(mutex_);
    return products_.size();
}

void ProductStore::evict_to_fit(
    std::size_t incoming_bytes,
    const std::string& replacing_id) {
    while ((!products_.empty() && bytes_ + incoming_bytes > maximum_total_bytes_) ||
           products_.size() >= maximum_products_ +
               (products_.contains(replacing_id) ? std::size_t{1} : std::size_t{0})) {
        auto oldest = products_.end();
        for (auto iterator = products_.begin(); iterator != products_.end(); ++iterator) {
            if (iterator->first == replacing_id) continue;
            if (oldest == products_.end() || iterator->second.age < oldest->second.age) {
                oldest = iterator;
            }
        }
        if (oldest == products_.end()) break;
        bytes_ -= oldest->second.product.size();
        products_.erase(oldest);
    }
}

}  // namespace dqm_gateway
