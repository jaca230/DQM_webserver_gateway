#include "dqm_gateway/product/data_product_registry.h"

#include "dqm_gateway/api/route/api_route_registry.h"
#include "dqm_gateway/model/product.h"
#include "dqm_gateway/model/stores/data_store.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace dqm_gateway {

void DataProductRegistry::add(std::unique_ptr<DataProduct> product) {
    if (!product) throw std::invalid_argument("Cannot register a null data product");
    const std::string name(product->name());
    if (name.empty()) throw std::invalid_argument("Data product name cannot be empty");
    for (const auto& registered : products_) {
        if (registered->name() == name) {
            throw std::invalid_argument("Duplicate data product: " + name);
        }
    }
    products_.push_back(std::move(product));
    std::stable_sort(products_.begin(), products_.end(), [](const auto& left, const auto& right) {
        return left->ingest_priority() < right->ingest_priority();
    });
}

bool DataProductRegistry::ingest(const Product& product) const {
    bool accepted = false;
    bool successful = true;
    for (const auto& data_product : products_) {
        if (!data_product->accepts(product)) continue;
        accepted = true;
        try {
            successful = data_product->ingest(product) && successful;
        } catch (const std::exception& exception) {
            std::cerr << "[product/" << data_product->name() << "] Product "
                      << product.id << " could not be ingested: "
                      << exception.what() << '\n';
            successful = false;
        }
    }
    return accepted && successful;
}

void DataProductRegistry::register_routes(ApiRouteRegistry& routes) const {
    for (const auto& product : products_) {
        product->register_routes(routes);
    }
}

nlohmann::json DataProductRegistry::catalog() const {
    nlohmann::json result{{"protocol", "dqm-gateway-v1"}};
    for (const auto& product : products_) {
        result[product->name()] = product->store().catalog_json();
    }
    return result;
}

DataStore* DataProductRegistry::find_store(const std::string& name) const noexcept {
    for (const auto& product : products_) {
        if (product->store().name() == name) return &product->store();
    }
    return nullptr;
}

}  // namespace dqm_gateway
