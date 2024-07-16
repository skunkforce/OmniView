#include "websocket_client.hpp"

void sendToWebSocket(const nlohmann::json& jsonData) {
    using namespace web;
    using namespace web::websockets::client;

    websocket_client client;
    client.connect(U("ws://localhost:8080/")).wait();

    websocket_outgoing_message msg;
    msg.set_utf8_message(jsonData.dump());
    client.send(msg).wait();
    client.close().wait();
}

nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& data) {
    nlohmann::json jsonData = nlohmann::json::array();

    for (const auto& entry : data) {
        const auto& value = entry.second;
        for (const auto& pair : value) {
            nlohmann::json dataPoint = {{"x", pair.first}, {"y", pair.second}};
            jsonData.push_back(dataPoint);
        }
    }
    return jsonData;
}
