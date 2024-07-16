#ifndef WEBSOCKETHANDLER_HPP
#define WEBSOCKETHANDLER_HPP

#include <cpprest/ws_client.h>
#include "jasonhandler.hpp"

class WebSocketHandler {
public:
    WebSocketHandler(const std::string& uri);
    ~WebSocketHandler();
    void send(const nlohmann::json& jsonData);
    void close();
private:
    web::websockets::client::websocket_client handler;
};

#endif
