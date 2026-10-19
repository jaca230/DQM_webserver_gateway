#include "dqm_gateway/product/waveform/waveform_json_decoder.h"

#include "dqm_gateway/product/waveform/waveform_store.h"

#include <nlohmann/json.hpp>

namespace dqm_gateway {

WaveformJsonDecoder::WaveformJsonDecoder(WaveformStore& store) : store_(store) {}

bool WaveformJsonDecoder::accepts(const Product& product) const {
    return product.schema == "dqm-waveform-v1" && product.content_type == "application/json";
}

void WaveformJsonDecoder::decode(const Product& product) {
    const auto* begin = reinterpret_cast<const char*>(product.payload->data());
    const auto document = nlohmann::json::parse(begin, begin + product.payload->size());
    const auto detector = document.at("detector").get<std::string>();
    const auto epoch = document.value("epoch", std::string{"online"});
    const auto generated_at = document.value("generated-at-unix-ns", std::int64_t{0});
    std::vector<WaveformSnapshot> decoded;
    for (const auto& item : document.at("waveforms")) {
        WaveformSnapshot waveform;
        waveform.detector = detector;
        waveform.epoch = item.value("epoch", epoch);
        waveform.module = item.value("module", 0);
        waveform.submodule = item.value("submodule", 0);
        waveform.channel = item.at("channel").get<int>();
        waveform.sample_period_ns = item.value("sample-period-ns", 0.0);
        waveform.sequence = item.value("sequence", product.product_sequence);
        waveform.generated_at_unix_ns = item.value("generated-at-unix-ns", generated_at);
        waveform.samples = item.at("samples").get<std::vector<double>>();
        if (item.contains("times-ns")) {
            waveform.times_ns = item.at("times-ns").get<std::vector<double>>();
        }
        decoded.push_back(std::move(waveform));
    }
    store_.update(std::move(decoded));
}

}  // namespace dqm_gateway
