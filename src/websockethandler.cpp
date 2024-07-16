#include "websockethandler.hpp"

WebSocketHandler::WebSocketHandler(const std::string& uri) {
    handler.connect(uri).wait();
}

WebSocketHandler::~WebSocketHandler() {
    handler.close().wait();
}

void WebSocketHandler::send(const nlohmann::json& jsonData) {
    web::websockets::client::websocket_outgoing_message msg;
    msg.set_utf8_message(jsonData.dump());
    handler.send(msg).wait();
}

void WebSocketHandler::close() {
    handler.close().wait();
}

