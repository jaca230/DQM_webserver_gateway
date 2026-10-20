#include "dqm_gateway/product/atar/atar_json_decoder.h"
#include "dqm_gateway/product/atar/atar_store.h"

#include <stdexcept>

namespace dqm_gateway {
bool AtarJsonDecoder::accepts(const Product& product) const {
    return product.schema == "dqm-atar-view-v1" && product.content_type == "application/json";
}
void AtarJsonDecoder::decode(const Product& product) {
    if (!product.payload) throw std::invalid_argument("ATAR product has no payload");
    const auto* begin = reinterpret_cast<const char*>(product.payload->data());
    auto value = nlohmann::json::parse(begin, begin + product.payload->size());
    if (value.value("schema", "") != "dqm-atar-view-v1")
        throw std::invalid_argument("invalid ATAR product schema");
    store_.update(std::move(value));
}
}  // namespace dqm_gateway
