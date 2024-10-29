#include "api_server.hpp"
#include <cpprest/ws_client.h>
#include <iostream>

ApiServer::ApiServer(const std::string& address) : listener_(http_listener(U(address))) {
    // Define endpoints
    listener_.support(methods::GET, std::bind(&ApiServer::handleGet, this, std::placeholders::_1));
}

// Start the Server
void ApiServer::start() {
    listener_.open().then([]() {
        std::cout << "API Server is running..." << std::endl;
    }).wait();
}

// Stop the Server
void ApiServer::stop() {
    listener_.close().wait();
}

// Handler for GET-Request
void ApiServer::handleGet(http_request request) {
    auto path = uri::decode(request.relative_uri().path());
    if (path == "/status") {
        // Responds with JSON data
        json::value response;
        response[U("status")] = json::value::string(U("API is up and running"));
        request.reply(status_codes::OK, response);
    }
    else if (path == "/check_websocket") {
        checkWebSocket(request);
    }
    else {
        request.reply(status_codes::NotFound, "Endpoint not found");
    }
}

// Function for testing the WebSocket connection
void ApiServer::checkWebSocket(const http_request& request) {
    auto query_params = uri::split_query(request.request_uri().query());
    auto ws_uri_iter = query_params.find(U("wsuri"));

    if (ws_uri_iter == query_params.end()) {
        request.reply(status_codes::BadRequest, "Missing wsuri parameter");
        return;
    }

    const std::string ws_uri = ws_uri_iter->second;

    web::websockets::client::websocket_client ws_client;
    try {
        ws_client.connect(ws_uri).wait();
        ws_client.close().wait();
        json::value response;
        response[U("status")] = json::value::string(U("WebSocket connection successful"));
        request.reply(status_codes::OK, response);
    }
    catch (const web::websockets::client::websocket_exception& e) {
        json::value response;
        response[U("status")] = json::value::string(U("WebSocket connection failed"));
        response[U("error")] = json::value::string(e.what());
        request.reply(status_codes::InternalError, response);
    }
}
