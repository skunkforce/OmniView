#include "api_server.hpp" // Einbindung des API-Servers
#include <iostream>

int main() {
    const std::string serverAddress = "http://localhost:8080";

    ApiServer apiServer(serverAddress);
    apiServer.start();

    std::cout << "Press ENTER to stop the server." << std::endl;
    std::cin.get();

    apiServer.stop();
    return 0;
}

