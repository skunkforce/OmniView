#include "ws.hpp"

std::vector<crow::websocket::connection*> connections;
std::mutex connections_mutex;

void configureWebSocketRoutes(crow::SimpleApp& app) {
    fmt::print("The WebSocket is started\n");
    CROW_WEBSOCKET_ROUTE(app, "/")
        .onopen([&](crow::websocket::connection& conn) {
            std::lock_guard<std::mutex> lock(connections_mutex);
            connections.push_back(&conn);
            fmt::print("WebSocket connection opended: {}\n", static_cast<void*>(&conn));
            
            })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
            std::lock_guard<std::mutex> lock(connections_mutex);
            connections.erase(std::remove(connections.begin(), connections.end(), &conn), connections.end());
            fmt::print("WebSocket connection closed: {}\n", static_cast<void*>(&conn));

            })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data,  bool is_bunary) {
            fmt::print("Received message: {}\n", data);

            // Forward the message to all connected clients
            std::lock_guard<std::mutex> lock(connections_mutex);
            for (auto& client_conn : connections) {
                if (client_conn != &conn) {
                    client_conn->send_text(data);
                }
            }
            });
}

void startWebSocketServer() {
    // define crow application
    crow::SimpleApp app;

    // Configure WebSocket routes 
    configureWebSocketRoutes(app);

    // Start the WebSocket Server
    app.port(8080)
        .multithreaded()
        .run();
}
