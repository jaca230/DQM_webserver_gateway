#include "dqm_gateway/api/context/api_context.h"

#include "dqm_gateway/metrics/gateway_metrics.h"

#include <drogon/HttpResponse.h>
#include <json/value.h>
#include <nlohmann/json.hpp>

namespace dqm_gateway {
namespace {

drogon::HttpStatusCode status_code(int value) {
    return static_cast<drogon::HttpStatusCode>(value);
}

}  // namespace

ApiContext::ApiContext(GatewayMetrics& metrics, const LimitConfig& limits)
    : metrics_(metrics),
      limits_(limits),
      rate_limiter_(limits.requests_per_second, limits.request_burst) {}

bool ApiContext::allow(const drogon::HttpRequestPtr& request, Callback& callback) {
    metrics_.record_http_request();
    if (rate_limiter_.allow(request->peerAddr().toIp())) return true;
    metrics_.record_rate_limited_request();
    auto response = drogon::HttpResponse::newHttpJsonResponse(Json::Value{});
    response->setStatusCode(drogon::k429TooManyRequests);
    response->addHeader("Retry-After", "1");
    response->setBody(R"({"error":"request rate limit exceeded"})");
    callback(response);
    return false;
}

void ApiContext::send_json(
    Callback& callback,
    const nlohmann::json& value,
    int status) const {
    auto body = value.dump();
    if (body.size() > limits_.maximum_response_bytes) {
        send_error(callback, 413, "response exceeds the configured byte limit");
        return;
    }
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(status_code(status));
    response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    response->addHeader("Cache-Control", "no-cache");
    response->setBody(std::move(body));
    callback(response);
}

void ApiContext::send_error(
    Callback& callback,
    int status,
    const std::string& message) const {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(status_code(status));
    response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    response->setBody(nlohmann::json{{"error", message}}.dump());
    callback(response);
}

}  // namespace dqm_gateway
