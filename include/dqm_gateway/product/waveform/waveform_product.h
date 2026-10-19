#pragma once

#include "dqm_gateway/product/waveform/waveform_store.h"
#include "dqm_gateway/product/data_product.h"
#include "dqm_gateway/product/waveform/waveform_json_decoder.h"

namespace dqm_gateway {

struct DataProductDependencies;

class WaveformProduct final : public DataProduct {
public:
    explicit WaveformProduct(const DataProductDependencies& dependencies);

    [[nodiscard]] std::string_view name() const noexcept override { return "waveforms"; }
    [[nodiscard]] bool accepts(const Product& product) const override;
    bool ingest(const Product& product) override;
    [[nodiscard]] DataStore& store() noexcept override { return store_; }
    void register_routes(ApiRouteRegistry& routes) override;

private:
    WaveformStore store_;
    WaveformJsonDecoder decoder_;
};

}  // namespace dqm_gateway
