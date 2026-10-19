#pragma once

#include "dqm_gateway/product/histogram/histogram_store.h"
#include "dqm_gateway/product/data_product.h"
#include "dqm_gateway/product/histogram/histogram_json_decoder.h"

namespace dqm_gateway {

struct DataProductDependencies;

class HistogramProduct final : public DataProduct {
public:
    explicit HistogramProduct(const DataProductDependencies& dependencies);

    [[nodiscard]] std::string_view name() const noexcept override { return "histograms"; }
    [[nodiscard]] bool accepts(const Product& product) const override;
    bool ingest(const Product& product) override;
    [[nodiscard]] DataStore& store() noexcept override { return store_; }
    void register_routes(ApiRouteRegistry& routes) override;

private:
    HistogramStore store_;
    HistogramJsonDecoder decoder_;
};

}  // namespace dqm_gateway
