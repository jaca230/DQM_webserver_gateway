#pragma once

#include "dqm_gateway/product/raw/product_store.h"
#include "dqm_gateway/product/data_product.h"

namespace dqm_gateway {

struct DataProductDependencies;

class RawProduct final : public DataProduct {
public:
    explicit RawProduct(const DataProductDependencies& dependencies);

    [[nodiscard]] std::string_view name() const noexcept override { return "products"; }
    [[nodiscard]] int ingest_priority() const noexcept override { return 1000; }
    [[nodiscard]] bool accepts(const Product&) const override { return true; }
    bool ingest(const Product& product) override;
    [[nodiscard]] DataStore& store() noexcept override { return store_; }
    void register_routes(ApiRouteRegistry& routes) override;

private:
    ProductStore store_;
};

}  // namespace dqm_gateway
