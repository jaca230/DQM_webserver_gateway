#include "dqm_gateway/product/waveform/routes/waveform_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/waveform/waveform_store.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>

namespace dqm_gateway {

WaveformRoute::WaveformRoute(WaveformStore& store, std::size_t maximum_samples)
    : DataRoute(store), waveforms_(store), maximum_samples_(maximum_samples) {}

void WaveformRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/waveforms",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto detector = request->getParameter("detector");
            if (detector.empty()) {
                return context.send_error(callback, 400, "detector is required");
            }
            try {
                context.send_json(callback, waveforms_.view(
                    detector,
                    optional_integer(request, "module"),
                    optional_integer(request, "submodule"),
                    optional_integer(request, "channel"),
                    bounded_size(request, "max-samples", maximum_samples_)));
            } catch (const std::exception& exception) {
                context.send_error(callback, 400, exception.what());
            }
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
