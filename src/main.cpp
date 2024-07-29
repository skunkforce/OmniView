#include "websockethandler.hpp"
#include "handler.hpp"
#include <thread>
#include <CLI/CLI.hpp>
#include <csignal>

int main(int argc, char** argv) {
    
    bool flagPaused{true};
    std::set<std::string> selected_serials;
    std::string wsURI;
    
    CLI::App app{"OmniView"};

    app.add_option("-w, --wsuri", wsURI, "WebSocket URI")->type_name("");

    CLI11_PARSE(app, argc, argv);
    
    WebSocketHandler wsHandler(wsURI);

    // Signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    // WebSocket handler thread
    std::thread webSocketThread([&]() {
        while (running) {
            std::cout << "F00 F00" << std::endl;
            if (sampler.has_value() && !flagPaused) {
                sampler->copyOut(captureData);
                wsHandler.send(captureData, selected_serials);
            }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    });

    webSocketThread.detach();

    return 0;
}
