#include "dqm_gateway/api/live_websocket_controller.h"

#include "dqm_gateway/api/websocket_hub.h"

#include <cstddef>
#include <exception>
#include <nlohmann/json.hpp>
#include <string>

namespace dqm_gateway {

WebsocketHub* LiveWebsocketController::hub_ = nullptr;

void LiveWebsocketController::configure_hub(WebsocketHub& hub) {
    hub_ = &hub;
}

void LiveWebsocketController::handleNewMessage(
    const drogon::WebSocketConnectionPtr& connection,
    std::string&& message,
    const drogon::WebSocketMessageType& type) {
    if (!hub_ || type != drogon::WebSocketMessageType::Text) return;
    if (message.size() > 64U * 1024U) {
        connection->shutdown(drogon::CloseCode::kViolation, "subscription request too large");
        return;
    }
    try {
        hub_->configure(connection, message);
        connection->send(R"({"type":"subscription-updated"})");
    } catch (const std::exception& exception) {
        connection->send(nlohmann::json{{"type", "error"}, {"message", exception.what()}}.dump());
    }
}

void LiveWebsocketController::handleNewConnection(
    const drogon::HttpRequestPtr& request,
    const drogon::WebSocketConnectionPtr& connection) {
    if (!hub_ || !hub_->accepts(request)) {
        connection->shutdown(drogon::CloseCode::kViolation, "origin is not allowed");
        return;
    }
    hub_->connected(connection);
}

void LiveWebsocketController::handleConnectionClosed(
    const drogon::WebSocketConnectionPtr& connection) {
    if (hub_) hub_->disconnected(connection);
}

}  // namespace dqm_gateway
