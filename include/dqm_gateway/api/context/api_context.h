#pragma once

#include "dqm_gateway/api/client_rate_limiter.h"
#include "dqm_gateway/config/sections/limit_config.h"

#include <functional>
#include <string>

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <nlohmann/json_fwd.hpp>

namespace dqm_gateway {

class GatewayMetrics;

class ApiContext final {
public:
    using Callback = std::function<void(const drogon::HttpResponsePtr&)>;

    ApiContext(GatewayMetrics& metrics, const LimitConfig& limits);
    bool allow(const drogon::HttpRequestPtr& request, Callback& callback);
    void send_json(Callback& callback, const nlohmann::json& value, int status = 200) const;
    void send_error(Callback& callback, int status, const std::string& message) const;

    [[nodiscard]] GatewayMetrics& metrics() const noexcept { return metrics_; }
    [[nodiscard]] const LimitConfig& limits() const noexcept { return limits_; }

private:
    GatewayMetrics& metrics_;
    const LimitConfig& limits_;
    ClientRateLimiter rate_limiter_;
};

}  // namespace dqm_gateway
