#include "websockethandler.hpp"

WebSocketHandler::WebSocketHandler(const std::string& uri) {
    handler.connect(uri).wait();
}

WebSocketHandler::~WebSocketHandler() {
    handler.close().wait();
}

void WebSocketHandler::send(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& dataMap, const std::set<std::string>& filter_serials) {
    auto jsonData = captureDataToJson(dataMap, filter_serials);
    web::websockets::client::websocket_outgoing_message msg;
    msg.set_utf8_message(jsonData.dump());
    handler.send(msg).wait();
}

void WebSocketHandler::close() {
    handler.close().wait();
}

nlohmann::json WebSocketHandler::captureDataToJson(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& dataMap, const std::set<std::string>& filter_serials) {
    nlohmann::json jsonData = nlohmann::json::array();

    for (const auto& entry : dataMap) {
        const Omniscope::Id& id = entry.first;
        if (!filter_serials.empty() && filter_serials.find(id.serial) == filter_serials.end()) {
            continue;
        }
        const auto& value = entry.second;
        for (const auto& pair : value) {
            nlohmann::json dataPoint = {{"Id", id.serial}, {"x", pair.first}, {"y", pair.second}};
            jsonData.push_back(dataPoint);
        }
    }
    return jsonData;
}
