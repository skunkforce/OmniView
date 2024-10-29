#include "api_server.hpp"
#include <iostream>

ApiServer::ApiServer(const std::string& address) : listener_(http_listener(U(address))) {
    // Definieren des `/status`-Endpunkts
    listener_.support(methods::GET, std::bind(&ApiServer::handleGet, this, std::placeholders::_1));
}

// Startet den Server
void ApiServer::start() {
    listener_.open().then([]() {
        std::cout << "API Server is running..." << std::endl;
    }).wait();
}

// Stoppt den Server
void ApiServer::stop() {
    listener_.close().wait();
}

// Handler für GET-Anfragen
void ApiServer::handleGet(http_request request) {
    auto path = uri::decode(request.relative_uri().path());
    if (path == "/status") {
        // Antwortet mit JSON-Daten
        json::value response;
        response[U("status")] = json::value::string(U("API is up and running"));
        request.reply(status_codes::OK, response);
    } else {
        request.reply(status_codes::NotFound, "Endpoint not found");
    }
}

