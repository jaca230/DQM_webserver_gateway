#include "dqm_gateway/api/route/system/catalog_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/data_product_registry.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>

namespace dqm_gateway {

void CatalogRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/catalog",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            context.send_json(callback, products_.catalog());
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
