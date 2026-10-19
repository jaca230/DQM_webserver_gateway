#include "dqm_gateway/product/raw/routes/product_payload_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/raw/product_store.h"

#include <drogon/HttpAppFramework.h>

namespace dqm_gateway {

ProductPayloadRoute::ProductPayloadRoute(ProductStore& store)
    : DataRoute(store), products_(store) {}

void ProductPayloadRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/products/payload",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto id = request->getParameter("id");
            if (id.empty()) return context.send_error(callback, 400, "id is required");
            const auto product = products_.get(id);
            if (!product) return context.send_error(callback, 404, "product not found");
            if (product->size() > context.limits().maximum_response_bytes) {
                return context.send_error(callback, 413, "product exceeds the response byte limit");
            }
            const auto etag = '"' + std::to_string(product->product_sequence) + '"';
            if (request->getHeader("if-none-match") == etag) {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k304NotModified);
                callback(response);
                return;
            }
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeString(product->content_type);
            response->addHeader("ETag", etag);
            response->addHeader("Cache-Control", "no-cache");
            response->setBody(std::string(
                reinterpret_cast<const char*>(product->payload->data()),
                product->payload->size()));
            callback(response);
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
