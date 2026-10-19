#pragma once

#include "dqm_gateway/product/product_decoder.h"

namespace dqm_gateway {

class HitSeriesStore;

class HitSeriesJsonDecoder final : public ProductDecoder {
public:
    explicit HitSeriesJsonDecoder(HitSeriesStore& store);
    bool accepts(const Product& product) const override;
    void decode(const Product& product) override;

private:
    HitSeriesStore& store_;
};

}  // namespace dqm_gateway
