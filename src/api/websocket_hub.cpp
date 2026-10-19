#include "dqm_gateway/api/websocket_hub.h"

#include "dqm_gateway/metrics/gateway_metrics.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dqm_gateway {

WebsocketHub::WebsocketHub(
    double maximum_rate_hz,
    std::vector<std::string> allowed_origins,
    GatewayMetrics& metrics)
    : maximum_rate_hz_(maximum_rate_hz),
      allowed_origins_(std::move(allowed_origins)),
      metrics_(metrics) {}

bool WebsocketHub::accepts(const drogon::HttpRequestPtr& request) const {
    const auto origin = request->getHeader("origin");
    return origin.empty() ||
        std::find(allowed_origins_.begin(), allowed_origins_.end(), origin) !=
            allowed_origins_.end();
}

void WebsocketHub::connected(const drogon::WebSocketConnectionPtr& connection) {
    std::lock_guard lock(mutex_);
    clients_.try_emplace(connection);
    metrics_.set_websocket_clients(clients_.size());
    connection->send(R"({"type":"connected","protocol":"dqm-gateway-v1"})");
}

void WebsocketHub::disconnected(const drogon::WebSocketConnectionPtr& connection) {
    std::lock_guard lock(mutex_);
    clients_.erase(connection);
    metrics_.set_websocket_clients(clients_.size());
}

void WebsocketHub::configure(
    const drogon::WebSocketConnectionPtr& connection,
    const std::string& message) {
    const auto value = nlohmann::json::parse(message);
    const auto requested_rate = value.value("maximum-rate-hz", 1.0);
    if (requested_rate <= 0.0) throw std::invalid_argument("maximum-rate-hz must be positive");
    const auto products = value.value("products", std::vector<std::string>{});
    if (products.size() > 128) throw std::invalid_argument("At most 128 products may be subscribed");

    std::lock_guard lock(mutex_);
    const auto found = clients_.find(connection);
    if (found == clients_.end()) return;
    found->second.products = {products.begin(), products.end()};
    found->second.rate_hz = std::min(requested_rate, maximum_rate_hz_);
}

void WebsocketHub::publish(const Product& product) {
    const auto now = std::chrono::steady_clock::now();
    const auto message = nlohmann::json{
        {"type", "product-updated"},
        {"product", product_summary(product)},
    }.dump();

    std::vector<drogon::WebSocketConnectionPtr> recipients;
    {
        std::lock_guard lock(mutex_);
        for (auto& [connection, client] : clients_) {
            if (!connection->connected()) continue;
            if (!client.products.empty() && !client.products.contains(product.id)) continue;
            const auto interval = std::chrono::duration<double>(1.0 / client.rate_hz);
            if (client.last_sent.time_since_epoch().count() != 0 &&
                now - client.last_sent < interval) {
                continue;
            }
            client.last_sent = now;
            recipients.push_back(connection);
        }
    }
    for (const auto& connection : recipients) connection->send(message);
}

}  // namespace dqm_gateway
