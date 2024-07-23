#ifndef WEBSOCKETHANDLER_HPP
#define WEBSOCKETHANDLER_HPP

#include "jasonhandler.hpp"
#include "handler.hpp"
#include <cpprest/ws_client.h>
#include <nlohmann/json.hpp>
#include <map>
#include <set>

class WebSocketHandler {
public:
    WebSocketHandler(const std::string& uri);
    ~WebSocketHandler();
    void send(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& dataMap, const std::set<std::string>& filter_serials = {});
    void close();
private:
    web::websockets::client::websocket_client handler;
    nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& dataMap, const std::set<std::string>& filter_serials);
};

#endif
