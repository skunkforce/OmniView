#ifndef WEBSOCKETHANDLER_HPP
#define WEBSOCKETHANDLER_HPP

#include "handler.hpp"
#include "jsonhandler.hpp"
#include <cpprest/ws_client.h>
#include <map>
#include <set>
#include <nlohmann/json.hpp>

class WebSocketHandler {

public:
    WebSocketHandler(const std::string& uri);
    ~WebSocketHandler();

    // Starts the WebSocket thread to send the device data
    void startWebSocketThreadForDevices(const std::set<std::string>& selected_serials);

    // Starts the WebSocket thread to send the DLL data
    void startWebSocketThreadForDll(const std::vector<int>& dllData);

    // Method for sending JSON data
    void send(const nlohmann::json& jsonData);

    // Sending the device data
    void sendDeviceData(const std::map<Omniscope::Id, std::vector<std::tuple<double, double, std::chrono::time_point<std::chrono::high_resolution_clock>>>>& dataMap, const std::set<std::string>& filter_serials = {});

    // Sending the DLL data
    void sendDllData(const std::vector<int>& data, size_t timestamp);

    // Method for closing the WebSocket
    void close();

private:
    web::websockets::client::websocket_client handler;
};

#endif
