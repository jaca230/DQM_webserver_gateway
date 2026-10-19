#pragma once

#include "dqm_gateway/product/data_product.h"

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class ApiRouteRegistry;
class DataStore;
class Product;

class DataProductRegistry final {
public:
    void add(std::unique_ptr<DataProduct> product);
    bool ingest(const Product& product) const;
    void register_routes(ApiRouteRegistry& routes) const;

    [[nodiscard]] nlohmann::json catalog() const;
    [[nodiscard]] DataStore* find_store(const std::string& name) const noexcept;

private:
    std::vector<std::unique_ptr<DataProduct>> products_;
};

}  // namespace dqm_gateway
