#pragma once

#include "dqm_gateway/product/hit_series/hit_series_store.h"
#include "dqm_gateway/product/data_product.h"
#include "dqm_gateway/product/hit_series/hit_series_json_decoder.h"

namespace dqm_gateway {

struct DataProductDependencies;

class HitSeriesProduct final : public DataProduct {
public:
    explicit HitSeriesProduct(const DataProductDependencies& dependencies);

    [[nodiscard]] std::string_view name() const noexcept override { return "hit-series"; }
    [[nodiscard]] bool accepts(const Product& product) const override;
    bool ingest(const Product& product) override;
    [[nodiscard]] DataStore& store() noexcept override { return store_; }
    void register_routes(ApiRouteRegistry& routes) override;

private:
    HitSeriesStore store_;
    HitSeriesJsonDecoder decoder_;
};

}  // namespace dqm_gateway
