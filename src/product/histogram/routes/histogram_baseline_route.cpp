#include "dqm_gateway/product/histogram/routes/histogram_baseline_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/histogram/histogram_store.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json.hpp>

namespace dqm_gateway {

HistogramBaselineRoute::HistogramBaselineRoute(HistogramStore& store)
    : DataRoute(store), histograms_(store) {}

void HistogramBaselineRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/histograms/baselines",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            if (request->method() == drogon::Options) {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k204NoContent);
                response->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
                response->addHeader("Access-Control-Allow-Headers", "Content-Type");
                callback(response);
                return;
            }
            if (request->body().size() > 64U * 1024U) {
                return context.send_error(callback, 413, "baseline request is too large");
            }
            try {
                const auto value = nlohmann::json::parse(request->body());
                const auto histogram = value.at("histogram").get<std::string>();
                const auto baseline = value.at("name").get<std::string>();
                if (!histograms_.create_baseline(histogram, baseline)) {
                    return context.send_error(callback, 404, "histogram not found");
                }
                context.send_json(
                    callback, {{"histogram", histogram}, {"baseline", baseline}}, 201);
            } catch (const std::length_error& exception) {
                context.send_error(callback, 507, exception.what());
            } catch (const std::exception& exception) {
                context.send_error(callback, 400, exception.what());
            }
        },
        {drogon::Post, drogon::Options});
}

}  // namespace dqm_gateway
