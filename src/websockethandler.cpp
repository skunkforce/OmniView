#include "websockethandler.hpp"

WebSocketClient::WebSocketClient(const std::string& uri) {
    client.connect(uri).wait();
}

WebSocketClient::~WebSocketClient() {
    client.close().wait();
}

void WebSocketClient::send(const nlohmann::json& jsonData) {
    web::websockets::client::websocket_outgoing_message msg;
    msg.set_utf8_message(jsonData.dump());
    client.send(msg).wait();
}

void WebSocketClient::close() {
    client.close().wait();
}

