#include "dqm_gateway/api/route/system/health_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/input/data_source_registry.h"
#include "dqm_gateway/model/stores/data_store.h"
#include "dqm_gateway/product/data_product_registry.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>

namespace dqm_gateway {

void HealthRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/health",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto healthy = sources_.healthy();
            const auto* products = products_.find_store("products");
            context.send_json(callback, {
                {"status", healthy ? "ok" : "degraded"},
                {"sources", sources_.status()},
                {"stored-products", products ? products->stored_items() : 0},
                {"stored-bytes", products ? products->stored_bytes() : 0},
            }, healthy ? 200 : 503);
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
