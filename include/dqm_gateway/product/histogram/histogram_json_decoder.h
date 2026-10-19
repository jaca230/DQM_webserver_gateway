#pragma once

#include "dqm_gateway/product/product_decoder.h"

namespace dqm_gateway {

class HistogramStore;

class HistogramJsonDecoder final : public ProductDecoder {
public:
    explicit HistogramJsonDecoder(HistogramStore& store);

    bool accepts(const Product& product) const override;
    void decode(const Product& product) override;

private:
    HistogramStore& store_;
};

}  // namespace dqm_gateway

