#ifndef WEBSOCKETHANDLER_HPP
#define WEBSOCKETHANDLER_HPP

#include "handler.hpp"
#include "jsonhandler.hpp"
#include <cpprest/ws_client.h>
#include <map>
#include <set>

class WebSocketHandler {
public:
    WebSocketHandler(const std::string& uri);
    ~WebSocketHandler();
    void startWebSocketThread(const std::set<std::string>& selected_serials);
    void send(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& dataMap, const std::set<std::string>& filter_serials = {});
    void close();
private:
    web::websockets::client::websocket_client handler;
};

#endif
