#include "dqm_gateway/api/route/system/metrics_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/metrics/gateway_metrics.h"
#include "dqm_gateway/model/stores/data_store.h"
#include "dqm_gateway/product/data_product_registry.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpResponse.h>

namespace dqm_gateway {

void MetricsRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/metrics",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto* products = products_.find_store("products");
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
            response->setBody(metrics_.prometheus(
                products ? products->stored_items() : 0,
                products ? products->stored_bytes() : 0));
            callback(response);
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
