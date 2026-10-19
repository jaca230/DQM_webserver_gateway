#include "dqm_gateway/product/histogram/routes/histogram_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/histogram/histogram_store.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace dqm_gateway {

HistogramRoute::HistogramRoute(HistogramStore& store)
    : DataRoute(store), histograms_(store) {}

void HistogramRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/histograms",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto name = request->getParameter("name");
            if (name.empty()) return context.send_error(callback, 400, "name is required");
            std::optional<std::string> baseline;
            if (const auto value = request->getParameter("baseline"); !value.empty()) {
                baseline = value;
            }
            try {
                context.send_json(callback, histograms_.view(
                    name,
                    baseline,
                    bounded_size(
                        request, "max-bins", context.limits().maximum_histogram_bins)));
            } catch (const std::out_of_range& exception) {
                context.send_error(callback, 404, exception.what());
            } catch (const std::logic_error& exception) {
                context.send_error(callback, 409, exception.what());
            } catch (const std::exception& exception) {
                context.send_error(callback, 400, exception.what());
            }
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
