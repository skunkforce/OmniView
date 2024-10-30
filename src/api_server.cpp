#include "api_server.hpp"
// #include "handler.hpp"
#include <cstdlib>
#include <cpprest/ws_client.h>
#include <filesystem>
#include <iostream>
#include <fmt/core.h>
// #include <sstream>

ApiServer::ApiServer(const std::string& address) : listener_(http_listener(U(address))) {
    // Define endpoints
    listener_.support(methods::GET, std::bind(&ApiServer::handleGet, this, std::placeholders::_1));
    listener_.support(methods::POST, std::bind(&ApiServer::handlePost, this, std::placeholders::_1));
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

// GET handler
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
    else if (path == "/search_dlls") {
        searchDllsEndpoint(request);
    }

/*
    // sehe Header-Datei
    else if (path == "/search_devices") {
        searchDevicesEndpoint(request);
    }
*/
    else {
        request.reply(status_codes::NotFound, "Endpoint not found");
    }
}

// POST handler
void ApiServer::handlePost(http_request request) {
    auto path = uri::decode(request.relative_uri().path());
    if (path == "/load_dll") {
        loadDllEndpoint(request);
    }
    else if (path == "/unload_dll") {
        unloadDllEndpoint(request);
    }
    else if (path == "/start_websocket") {
        startWebSocket(request);
    }
    else if (path == "/stop_websocket") {
        stopWebSocket(request);
    }
    else {
        request.reply(status_codes::NotFound, "Endpoint not found");
    }
}

// #################### Functions ####################

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

// Function for searching for DLLs
void ApiServer::searchDllsEndpoint(const http_request& request) {
    auto query_params = uri::split_query(request.request_uri().query());
    auto path_iter = query_params.find(U("path"));
    
    if (path_iter == query_params.end()) {
        request.reply(status_codes::BadRequest, "Missing path parameter");
        return;
    }

    const std::string search_path = path_iter->second;
    json::value response = json::value::array();
    size_t index = 0;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(search_path)) {
            if (entry.path().extension() == ".so") {
                response[index++] = json::value::string(entry.path().string());
            }
        }
        if (index == 0) {
            request.reply(status_codes::NotFound, "No DLL files found");
        } else {
            request.reply(status_codes::OK, response);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        json::value errorResponse;
        errorResponse[U("error")] = json::value::string(e.what());
        request.reply(status_codes::InternalError, errorResponse);
    }
}

// Function for loading the DLL via the WebSocket server
void ApiServer::loadDllEndpoint(const http_request& request) {
    std::cout << "loadDll endpoint called" << std::endl;
    request.extract_json().then([=](json::value jsonData) {
        if (!jsonData.has_field(U("dllPaths")) || !jsonData[U("dllPaths")].is_array()) {
            request.reply(status_codes::BadRequest, "Missing dllPaths parameter or it is not an array");
            return;
        }

        // Convert DLL overlay std::vector<json::value>
        std::vector<json::value> dllPathVector;
        for (const auto& dllPath : jsonData[U("dllPaths")].as_array()) {
            dllPathVector.push_back(dllPath);
        }

        std::string wsUri = "ws://127.0.0.1:8081";
        if (jsonData.has_field(U("wsUri"))) {
            wsUri = jsonData[U("wsUri")].as_string();
        }

        std::cout << "Attempting to connect to WebSocket server at " << wsUri << std::endl;
        web::websockets::client::websocket_client ws_client;

        try {
            ws_client.connect(wsUri).wait();
            std::cout << "WebSocket connection established to " << wsUri << std::endl;

            // Create and send JSON message with all DLL paths
            json::value message;
            message[U("dllPaths")] = json::value::array(dllPathVector);

            web::websockets::client::websocket_outgoing_message msg;
            msg.set_utf8_message(message.serialize());

            std::cout << "Sending DLL paths message: " << message.serialize() << std::endl;
            ws_client.send(msg).wait();
            std::cout << "Message sent successfully." << std::endl;

            ws_client.close().wait();

            json::value response;
            response[U("status")] = json::value::string(U("DLL paths sent successfully"));
            request.reply(status_codes::OK, response);
        }
        catch (const web::websockets::client::websocket_exception& e) {
            std::cout << "Failed to connect or send message: " << e.what() << std::endl;
            json::value response;
            response[U("status")] = json::value::string(U("Failed to send DLL paths"));
            response[U("error")] = json::value::string(e.what());
            request.reply(status_codes::InternalError, response);
        }
    }).wait();
}

// Function for unloading the DLL from the WebSocket server
void ApiServer::unloadDllEndpoint(const http_request& request) {
    std::cout << "Unload DLL endpoint called" << std::endl;

    request.extract_json().then([=](json::value jsonData) {
        if (!jsonData.has_field(U("dllPaths")) || !jsonData[U("dllPaths")].is_array()) {
            request.reply(status_codes::BadRequest, "Missing dllPaths parameter or it is not an array");
            return;
        }

        // Collect DLL paths
        std::vector<json::value> dllPathVector;
        for (const auto& dllPath : jsonData[U("dllPaths")].as_array()) {
            dllPathVector.push_back(dllPath);
        }

        std::string wsUri = "ws://127.0.0.1:8081"; // Standard-WebSocket-URI
        if (jsonData.has_field(U("wsUri"))) {
            wsUri = jsonData[U("wsUri")].as_string();
        }

        // Connect to the WebSocket server
        web::websockets::client::websocket_client ws_client;

        try {
            ws_client.connect(wsUri).wait();

            // Send JSON message to unload the DLLs
            json::value message;
            message[U("unloadDllPaths")] = json::value::array(dllPathVector);

            web::websockets::client::websocket_outgoing_message msg;
            msg.set_utf8_message(message.serialize());

            ws_client.send(msg).wait();
            ws_client.close().wait();

            json::value response;
            response[U("status")] = json::value::string(U("DLL paths sent for unloading"));
            request.reply(status_codes::OK, response);
        }
        catch (const web::websockets::client::websocket_exception& e) {
            json::value response;
            response[U("status")] = json::value::string(U("Failed to send DLL paths for unloading"));
            response[U("error")] = json::value::string(e.what());
            request.reply(status_codes::InternalError, response);
        }
    }).wait();
}

// Starts the WebSocket server by executing the provided path to `wsDll` executable.
void ApiServer::startWebSocket(const http_request& request) {
    request.extract_json().then([this, &request](json::value jsonData) {
        if (!jsonData.has_field(U("path"))) {
            request.reply(status_codes::BadRequest, "Missing path parameter for WebSocket executable");
            return;
        }

        std::string wsPath = jsonData[U("path")].as_string();
        if (isWebSocketRunning.load()) {
            request.reply(status_codes::BadRequest, "WebSocket server is already running");
            return;
        }

        webSocketThread = std::thread([wsPath, this]() {
            std::string command = wsPath + " &";
            std::cout << "Starting WebSocket server with command: " << command << std::endl;
            std::system(command.c_str());

            // Short delay and check whether the process is running
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (std::system("pgrep -f wsDll") == 0) {
                std::cout << "WebSocket server started successfully." << std::endl;
                isWebSocketRunning = true;
            }
            else {
                std::cerr << "Failed to start WebSocket server." << std::endl;
                isWebSocketRunning = false;
            }
        });

        isWebSocketRunning = true;
        request.reply(status_codes::OK, "WebSocket server started successfully");
    }).wait();
}

// Stop the WebSocket server by terminating the thread
void ApiServer::stopWebSocket(const http_request& request) {
    if (!isWebSocketRunning.load()) {
        request.reply(status_codes::BadRequest, "WebSocket server is not running");
        return;
    }

    if (webSocketThread.joinable()) {
        std::cout << "Stopping WebSocket server..." << std::endl;
        std::system("pkill -f wsDll");
        webSocketThread.join();
        isWebSocketRunning = false;
        request.reply(status_codes::OK, "WebSocket server stopped successfully");
    }
    else {
        request.reply(status_codes::InternalError, "Failed to stop WebSocket server");
    }
}


/*
    // Sehe Header-Datei
// Function for searching for devices and returning them as JSON
void ApiServer::searchDevicesEndpoint(const http_request& request) {
    // Geräte suchen und stoppen
    searchDevices();

    // Antwort vorbereiten
    json::value response = json::value::array();
    size_t index = 0;

    if (devices.empty()) {
        response[index++][U("message")] = json::value::string("No devices found.");
    } else {
        for (const auto& device : devices) {
            json::value deviceInfo;
            std::ostringstream oss;
            oss << device->getId()->serial;
            deviceInfo[U("id")] = json::value::string(oss.str());
            deviceInfo[U("status")] = json::value::string("Found device");
            response[index++] = deviceInfo;
        }
    }

    // Antwort senden
    request.reply(status_codes::OK, response);
}
*/
