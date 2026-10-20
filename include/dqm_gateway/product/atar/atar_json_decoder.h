#pragma once

#include "dqm_gateway/product/product_decoder.h"

namespace dqm_gateway {
class AtarStore;
class AtarJsonDecoder final : public ProductDecoder {
public:
    explicit AtarJsonDecoder(AtarStore& store) : store_(store) {}
    [[nodiscard]] bool accepts(const Product& product) const override;
    void decode(const Product& product) override;
private:
    AtarStore& store_;
};
}  // namespace dqm_gateway
