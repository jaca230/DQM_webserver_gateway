#pragma once

#include "dqm_gateway/product/atar/atar_json_decoder.h"
#include "dqm_gateway/product/atar/atar_store.h"
#include "dqm_gateway/product/data_product.h"

namespace dqm_gateway {
struct DataProductDependencies;
class AtarProduct final : public DataProduct {
public:
    explicit AtarProduct(const DataProductDependencies& dependencies);
    [[nodiscard]] std::string_view name() const noexcept override { return "atar"; }
    [[nodiscard]] bool accepts(const Product& product) const override;
    bool ingest(const Product& product) override;
    [[nodiscard]] DataStore& store() noexcept override { return store_; }
    void register_routes(ApiRouteRegistry& routes) override;
private:
    AtarStore store_;
    AtarJsonDecoder decoder_;
};
}  // namespace dqm_gateway
