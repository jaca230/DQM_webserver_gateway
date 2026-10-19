#include "dqm_gateway/product/hit_series/routes/hit_series_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/hit_series/hit_series_store.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>

namespace dqm_gateway {

HitSeriesRoute::HitSeriesRoute(HitSeriesStore& store, std::size_t maximum_hits)
    : DataRoute(store), hit_series_(store), maximum_hits_(maximum_hits) {}

void HitSeriesRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/hit-series",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto detector = request->getParameter("detector");
            if (detector.empty()) {
                return context.send_error(callback, 400, "detector is required");
            }
            try {
                context.send_json(callback, hit_series_.view(
                    detector,
                    optional_integer(request, "module"),
                    optional_integer(request, "submodule"),
                    optional_integer(request, "channel"),
                    bounded_size(request, "max-hits", maximum_hits_)));
            } catch (const std::exception& exception) {
                context.send_error(callback, 400, exception.what());
            }
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
