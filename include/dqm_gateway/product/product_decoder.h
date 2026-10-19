#pragma once

#include "dqm_gateway/model/product.h"

namespace dqm_gateway {

class ProductDecoder {
public:
    virtual ~ProductDecoder() = default;
    [[nodiscard]] virtual bool accepts(const Product& product) const = 0;
    virtual void decode(const Product& product) = 0;
};

}  // namespace dqm_gateway
