#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <cpprest/ws_client.h>
#include <nlohmann/json.hpp>

class WebSocketClient {
public:
    WebSocketClient(const std::string& uri);
    ~WebSocketClient();
    void send(const nlohmann::json& jsonData);
    void close();
private:
    web::websockets::client::websocket_client client;
};

#endif
