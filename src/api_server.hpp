#ifndef API_SERVER_HPP
#define API_SERVER_HPP

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <thread>
#include <atomic>
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

    std::atomic<bool> isWebSocketRunning{false};
    std::thread webSocketThread;

    // Handler for GET-Requests
    void handleGet(http_request request);

    // Handler for POST-Requests
    void handlePost(http_request request);

    // Function for testing the WebSocket connection
    void checkWebSocket(const http_request& request);

    // Function for searching for DLLs
    void searchDllsEndpoint(const http_request& request);

    // Function for loading the DLL via the WebSocket server
    void loadDllEndpoint(const http_request& request);

    // Starts the WebSocket server
    void startWebSocket(const http_request& request);

    // Stops the WebSocket server
    void stopWebSocket(const http_request& request);

/*  
    // Konflikt mit fmt aus der OmniscopeSampler.hpp. Nicht lösbar ind er Kurzen Zeit
    // Function for searching for devices
    void searchDevicesEndpoint(const http_request& request);
*/
};

#endif // API_SERVER_HPP

