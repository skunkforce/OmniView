#include "crow.h"
#include "fmt/core.h"

void configureWebSocketRoutes(crow::SimpleApp& app) {
    fmt::print("The WebSocket is started\n");
    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([&](crow::websocket::connection& conn) {
            fmt::print("WebSocket connection opended: {}\n", static_cast<void*>(&conn));
            
            })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
            fmt::print("WebSocket connection closed: {}\n", static_cast<void*>(&conn));

            })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data,  bool is_bunary) {
            fmt::print("Received message: {}\n", data);
            });
}


int main() {
    // define crow application
    crow::SimpleApp app;

    // Configure WebSocket routes 
    configureWebSocketRoutes(app);

    // Start the WebSocket Server
    app.port(8080)
        .multithreaded()
        .run();

    return 0;

}
