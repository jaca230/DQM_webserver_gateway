#pragma once

#include "dqm_gateway/model/product.h"
#include "dqm_gateway/model/stores/data_store.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dqm_gateway {

class ProductStore final : public DataStore {
public:
    using UpdateCallback = std::function<void(const Product&)>;

    ProductStore(std::size_t maximum_total_bytes, std::size_t maximum_products);

    bool put(Product product);
    std::optional<Product> get(const std::string& id) const;
    std::vector<Product> catalog() const;
    void set_update_callback(UpdateCallback callback);

    std::size_t bytes() const;
    std::size_t size() const;
    [[nodiscard]] std::string_view name() const noexcept override { return "products"; }
    [[nodiscard]] nlohmann::json catalog_json() const override;
    [[nodiscard]] std::size_t stored_items() const override { return size(); }
    [[nodiscard]] std::size_t stored_bytes() const override { return bytes(); }

private:
    struct Entry {
        Product product;
        std::uint64_t age;
    };

    void evict_to_fit(std::size_t incoming_bytes, const std::string& replacing_id);

    const std::size_t maximum_total_bytes_;
    const std::size_t maximum_products_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, Entry> products_;
    std::size_t bytes_{0};
    std::uint64_t age_{0};
    UpdateCallback update_callback_;
};

}  // namespace dqm_gateway

