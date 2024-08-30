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
                std::map<Omniscope::Id, std::vector<std::pair<double, double>>> tempData;
                sampler->copyOut(tempData);
                captureData.clear();
                for (auto& [id, data_vector] : tempData) {
                    std::vector<std::tuple<double, double, std::chrono::time_point<std::chrono::high_resolution_clock>>> extended_data_vector;

                    for (auto& data_point : data_vector) {
                        auto now = std::chrono::high_resolution_clock::now();
                        extended_data_vector.emplace_back(data_point.first, data_point.second, now);

                        // Optional: Simulate a short delay (e.g. 1 microsecond)
                        // std::this_thread::sleep_for(std::chrono::microseconds(1));
                    }
                    captureData[id] = std::move(extended_data_vector);
                }
                send(captureData, selected_serials);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });
    webSocketThread.detach();
}

void WebSocketHandler::send(const std::map<Omniscope::Id, std::vector<std::tuple<double, double, std::chrono::time_point<std::chrono::high_resolution_clock>>>>& dataMap, const std::set<std::string>& filter_serials) {
    auto jsonData = captureDataToJson(dataMap, filter_serials);
    web::websockets::client::websocket_outgoing_message msg;
    msg.set_utf8_message(jsonData.dump());
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
