#pragma once

#include <string_view>

namespace dqm_gateway {

class ApiRouteRegistry;
class DataStore;
class Product;

class DataProduct {
public:
    virtual ~DataProduct() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual int ingest_priority() const noexcept { return 0; }
    [[nodiscard]] virtual bool accepts(const Product& product) const = 0;
    virtual bool ingest(const Product& product) = 0;
    [[nodiscard]] virtual DataStore& store() noexcept = 0;
    virtual void register_routes(ApiRouteRegistry& routes) = 0;
};

}  // namespace dqm_gateway
