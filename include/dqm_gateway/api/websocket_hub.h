#pragma once

#include "dqm_gateway/model/product.h"

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <drogon/HttpRequest.h>
#include <drogon/WebSocketConnection.h>

namespace dqm_gateway {

class GatewayMetrics;

class WebsocketHub {
public:
    WebsocketHub(
        double maximum_rate_hz,
        std::vector<std::string> allowed_origins,
        GatewayMetrics& metrics);

    bool accepts(const drogon::HttpRequestPtr& request) const;
    void connected(const drogon::WebSocketConnectionPtr& connection);
    void disconnected(const drogon::WebSocketConnectionPtr& connection);
    void configure(
        const drogon::WebSocketConnectionPtr& connection,
        const std::string& message);
    void publish(const Product& product);

private:
    struct Client {
        std::set<std::string> products;
        double rate_hz{1.0};
        std::chrono::steady_clock::time_point last_sent{};
    };

    const double maximum_rate_hz_;
    const std::vector<std::string> allowed_origins_;
    GatewayMetrics& metrics_;
    std::mutex mutex_;
    std::map<
        drogon::WebSocketConnectionPtr,
        Client,
        std::owner_less<drogon::WebSocketConnectionPtr>> clients_;
};

}  // namespace dqm_gateway
