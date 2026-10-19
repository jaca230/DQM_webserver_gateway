#include "dqm_gateway/product/hit_series/hit_series_json_decoder.h"

#include "dqm_gateway/product/hit_series/hit_series_store.h"

#include <nlohmann/json.hpp>

namespace dqm_gateway {

HitSeriesJsonDecoder::HitSeriesJsonDecoder(HitSeriesStore& store) : store_(store) {}

bool HitSeriesJsonDecoder::accepts(const Product& product) const {
    return product.schema == "dqm-hit-series-v1" &&
        product.content_type == "application/json";
}

void HitSeriesJsonDecoder::decode(const Product& product) {
    const auto* begin = reinterpret_cast<const char*>(product.payload->data());
    const auto document = nlohmann::json::parse(begin, begin + product.payload->size());
    const auto detector = document.at("detector").get<std::string>();
    const auto epoch = document.value("epoch", std::string{"online"});
    const auto generated_at = document.value("generated-at-unix-ns", std::int64_t{0});
    std::vector<HitSeriesSnapshot> decoded;
    for (const auto& value : document.at("hit-series")) {
        HitSeriesSnapshot item;
        item.detector = detector;
        item.epoch = value.value("epoch", epoch);
        item.module = value.value("module", 0);
        item.submodule = value.value("submodule", 0);
        item.channel = value.at("channel").get<int>();
        item.sequence = value.value("sequence", product.product_sequence);
        item.generated_at_unix_ns = value.value("generated-at-unix-ns", generated_at);
        item.times_ns = value.at("times-ns").get<std::vector<double>>();
        item.energies = value.at("energies").get<std::vector<double>>();
        decoded.push_back(std::move(item));
    }
    store_.update(std::move(decoded));
}

}  // namespace dqm_gateway
