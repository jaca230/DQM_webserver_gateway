#pragma once

#include <drogon/WebSocketController.h>

namespace dqm_gateway {

class WebsocketHub;

class LiveWebsocketController final
    : public drogon::WebSocketController<LiveWebsocketController> {
public:
    static void configure_hub(WebsocketHub& hub);

    void handleNewMessage(
        const drogon::WebSocketConnectionPtr& connection,
        std::string&& message,
        const drogon::WebSocketMessageType& type) override;
    void handleNewConnection(
        const drogon::HttpRequestPtr& request,
        const drogon::WebSocketConnectionPtr& connection) override;
    void handleConnectionClosed(
        const drogon::WebSocketConnectionPtr& connection) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/api/v1/stream");
    WS_PATH_LIST_END

private:
    static WebsocketHub* hub_;
};

}  // namespace dqm_gateway

