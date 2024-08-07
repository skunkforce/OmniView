#include "websockethandler.hpp"
#include "jsonhandler.hpp"

WebSocketHandler::WebSocketHandler(const std::string& uri) {
    try {
        handler.connect(uri).wait();
    }
    catch (const web::websockets::client::websocket_exception& e) {
        std::cerr << "Failed to connect to WebSocket: " << e.what() << std::endl;
        throw;
    }
}

WebSocketHandler::~WebSocketHandler() {
    try {
        handler.close().wait();
    }
    catch (const web::websockets::client::websocket_exception& e) {
        std::cerr << "Failed to close WebSocket: " << e.what() << std::endl;
    }
}

void WebSocketHandler::startWebSocketThread(const std::set<std::string>& selected_serials) {
    std::thread webSocketThread([&]() {
        while (running) {
            if (sampler.has_value()) {
                sampler->copyOut(captureData);
                send(captureData, selected_serials);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    webSocketThread.detach();
}

void WebSocketHandler::send(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& dataMap, const std::set<std::string>& filter_serials) {
    auto jsonData = captureDataToJson(dataMap, filter_serials);

    // DEBUG: Add line breaks between JSON objects
    std::string formattedData;
    for (const auto& item : jsonData) {
        formattedData += item.dump() + "\n";
    }

    web::websockets::client::websocket_outgoing_message msg;
    // msg.set_utf8_message(jsonData.dump());

    // DEBUG: send formattedData
    msg.set_utf8_message(formattedData);
    try {
        handler.send(msg).wait();
    }
    catch (const web::websockets::client::websocket_exception& e) {
        std::cerr << "Failed to send message: " << e.what() << std::endl;
    }
}

void WebSocketHandler::close() {
    try {
        handler.close().wait();
    }
    catch (const web::websockets::client::websocket_exception& e) {
        std::cerr << "Failed to close WebSocket: " << e.what() << std::endl;
    }
}
