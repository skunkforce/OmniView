#include "websockethandler.hpp"
#include "handler.hpp"
#include "commandLineParser.hpp"
#include <thread>
#include <csignal>

int main(int argc, char** argv) {

    std::set<std::string> selected_serials;
    CommandLineOptions options;

    // Parse command line arguments
    parseCommandLineArguments(argc, argv, options);

    if (options.search) {
        searchDevices();
        return 0;
    }

    // Signal handler for SIGINT
    setupSignalHandlers();

    WebSocketHandler wsHandler(options.wsURI);

    // Initialize devices;
    initializeDevices();

    // Select devices based on command line options
    if (!selectDevices(options, selected_serials)) {
        return 1;
    }

    // Automatically start data acquisition
    if (!devices.empty()) {
        if (!sampler.has_value()) {
            sampler.emplace(deviceManager, std::move(devices));
        }
    }

    // Start WebSocket handler thread
    wsHandler.startWebSocketThread(selected_serials);

    // Main loop
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
