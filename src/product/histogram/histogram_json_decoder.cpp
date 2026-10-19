#include "dqm_gateway/product/histogram/histogram_json_decoder.h"

#include "dqm_gateway/product/histogram/histogram_store.h"

#include <cstddef>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace dqm_gateway {

HistogramJsonDecoder::HistogramJsonDecoder(HistogramStore& store) : store_(store) {}

bool HistogramJsonDecoder::accepts(const Product& product) const {
    return product.schema == "dqm-histogram-v1" &&
           product.content_type == "application/json";
}

void HistogramJsonDecoder::decode(const Product& product) {
    if (!product.payload) throw std::invalid_argument("Histogram product has no payload");
    const auto* begin = reinterpret_cast<const char*>(product.payload->data());
    const auto value = nlohmann::json::parse(begin, begin + product.payload->size());
    const auto& histograms = value.contains("histograms")
        ? value.at("histograms")
        : value;
    if (!histograms.is_array()) {
        throw std::invalid_argument("dqm-histogram-v1 payload must be an array or contain histograms");
    }

    const auto default_epoch = value.is_object()
        ? value.value("epoch", std::string{"unspecified"})
        : std::string{"unspecified"};
    const auto default_definition = value.is_object()
        ? value.value("definition", std::string{"unspecified"})
        : std::string{"unspecified"};
    const auto generated_at = value.is_object()
        ? value.value("generated-at-unix-ns", std::int64_t{0})
        : std::int64_t{0};
    std::vector<HistogramSnapshot> decoded;
    decoded.reserve(histograms.size());
    for (const auto& item : histograms) {
        HistogramSnapshot histogram;
        histogram.name = item.at("name").get<std::string>();
        histogram.title = item.value("title", histogram.name);
        histogram.units = item.value("units", std::string{});
        histogram.definition = item.value("definition", default_definition);
        histogram.epoch = item.value("epoch", default_epoch);
        histogram.sequence = item.value("sequence", product.product_sequence);
        histogram.generated_at_unix_ns = item.value("generated-at-unix-ns", generated_at);
        histogram.edges = item.at("edges").get<std::vector<double>>();
        histogram.counts = item.at("counts").get<std::vector<double>>();
        if (item.contains("sumw2")) {
            histogram.sumw2 = item.at("sumw2").get<std::vector<double>>();
        }
        decoded.push_back(std::move(histogram));
    }
    store_.update(std::move(decoded));
}

}  // namespace dqm_gateway
