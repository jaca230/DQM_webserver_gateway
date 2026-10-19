#pragma once

#include "dqm_gateway/product/product_decoder.h"

namespace dqm_gateway {

class WaveformStore;

class WaveformJsonDecoder final : public ProductDecoder {
public:
    explicit WaveformJsonDecoder(WaveformStore& store);
    bool accepts(const Product& product) const override;
    void decode(const Product& product) override;

private:
    WaveformStore& store_;
};

}  // namespace dqm_gateway
