#include "dqm_gateway/product/atar/routes/atar_route.h"
#include "dqm_gateway/api/context/api_context.h"
#include "dqm_gateway/product/atar/atar_store.h"

#include <drogon/HttpAppFramework.h>
namespace dqm_gateway {
AtarRoute::AtarRoute(AtarStore& store) : DataRoute(store), atar_(store) {}
void AtarRoute::register_route(ApiContext& context) {
    drogon::app().registerHandler("/api/v1/atar",
        [this, &context](const drogon::HttpRequestPtr& request, ApiContext::Callback&& callback) {
            if (!context.allow(request, callback)) return;
            const auto view = request->getParameter("view");
            if (view.empty()) return context.send_error(callback, 400, "view is required");
            try { context.send_json(callback, atar_.view(view)); }
            catch (const std::exception& exception) { context.send_error(callback, 400, exception.what()); }
        }, {drogon::Get});
}
}  // namespace dqm_gateway
