#include "dqm_gateway/product/raw/routes/product_metadata_route.h"

#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/model/product.h"
#include "dqm_gateway/product/raw/product_store.h"

#include <drogon/HttpAppFramework.h>

namespace dqm_gateway {

ProductMetadataRoute::ProductMetadataRoute(ProductStore& store)
    : DataRoute(store), products_(store) {}

void ProductMetadataRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler(
        "/api/v1/products/metadata",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto id = request->getParameter("id");
            if (id.empty()) return context.send_error(callback, 400, "id is required");
            const auto product = products_.get(id);
            if (!product) return context.send_error(callback, 404, "product not found");
            auto value = product_summary(*product);
            value["source-metadata"] = product->metadata;
            context.send_json(callback, value);
        },
        {drogon::Get});
}

}  // namespace dqm_gateway
