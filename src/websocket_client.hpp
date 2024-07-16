#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <cpprest/ws_client.h>
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
#include <fmt/core.h>
#include "style.hpp"
#include "jasonhandler.hpp"

void sendToWebSocket(const nlohmann::json& jsonData);
nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& data);

#endif
