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

    // Startet den Server
    void start();

    // Stoppt den Server
    void stop();

private:
    http_listener listener_;

    // Handler für GET-Anfragen
    void handleGet(http_request request);
};

#endif // API_SERVER_HPP

