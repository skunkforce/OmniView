#ifndef API_SERVER_HPP
#define API_SERVER_HPP

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <string>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class ApiServer {
public:
    explicit ApiServer(const std::string& address);

    // Start the Server
    void start();

    // Stop the Server
    void stop();

private:
    http_listener listener_;

    // Handler for GET-Request
    void handleGet(http_request request);

    // Function for testing the WebSocket connection
    void checkWebSocket(const http_request& request);
};

#endif // API_SERVER_HPP

