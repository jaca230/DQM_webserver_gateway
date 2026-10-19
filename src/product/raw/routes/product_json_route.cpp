#include "dqm_gateway/product/raw/routes/product_json_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/raw/product_store.h"

#include <drogon/HttpAppFramework.h>
#include <nlohmann/json.hpp>

namespace dqm_gateway {

ProductJsonRoute::ProductJsonRoute(ProductStore& store)
    : DataRoute(store), products_(store) {}

void ProductJsonRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/products/json",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto id = request->getParameter("id");
            if (id.empty()) return context.send_error(callback, 400, "id is required");
            const auto product = products_.get(id);
            if (!product) return context.send_error(callback, 404, "product not found");
            if (product->content_type != "application/json") {
                return context.send_error(callback, 415, "product is not JSON");
            }
            if (product->size() > context.limits().maximum_response_bytes) {
                return context.send_error(callback, 413, "product exceeds the response byte limit");
            }
            try {
                const auto* begin = reinterpret_cast<const char*>(product->payload->data());
                context.send_json(
                    callback,
                    nlohmann::json::parse(begin, begin + product->payload->size()));
            } catch (const std::exception& exception) {
                context.send_error(callback, 422, exception.what());
            }
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
