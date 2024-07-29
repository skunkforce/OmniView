#include "websockethandler.hpp"
#include "handler.hpp"
#include <thread>
#include <CLI/CLI.hpp>
#include <csignal>

int main(int argc, char** argv) {
    
    bool flagPaused{true};
    std::set<std::string> selected_serials;
    std::string wsURI;
    std::string deviceId;
    bool search = false;
    
    CLI::App app{"OmniView"};

    app.add_option("-w, --wsuri", wsURI, "WebSocket URI")->type_name("");
    app.add_option("-d, --device", deviceId, "Omniscope Device ID")->type_name("");
    app.add_flag("--search", search, "Search for devices");

    CLI11_PARSE(app, argc, argv);

    if (search) {
        searchDevices();
        return 0;
    }

    WebSocketHandler wsHandler(wsURI);

    // Signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (!deviceId.empty()) {
        startDevice(deviceId, flagPaused);
    }

    // Start console handler thread
    //std::thread consoleThread(consoleHandler, std::ref(flagPaused), std::ref(selected_serials));

    // WebSocket handler thread
    std::thread webSocketThread([&]() {
        while (running) {
            if (sampler.has_value() && !flagPaused) {
            sampler->copyOut(captureData);
            wsHandler.send(captureData, selected_serials);
        }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    });

    //consoleThread.join();
    webSocketThread.detach();

    return 0;
}
