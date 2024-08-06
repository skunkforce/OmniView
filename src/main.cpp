#include "websockethandler.hpp"
#include "handler.hpp"
#include "commandLineParser.hpp"
#include <thread>
#include <csignal>

int main(int argc, char** argv) {
    
    bool flagPaused{true};
    std::set<std::string> selected_serials;
    CommandLineOptions options;

    // Parse command line arguments
    parseCommandLineArguments(argc, argv, options);

    if (options.search) {
        searchDevices();
        stopAllDevices();
        return 0;
    }

    // Signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    WebSocketHandler wsHandler(options.wsURI);

    // Initialize devices;
    devices.clear();
    deviceManager.clearDevices();
    initDevices();

    // Check if the specified device is available
    if (!devices.empty()) {
        bool deviceFound = false;
        for (const auto& device : devices) {
            if (device->getId()->serial == options.deviceId) {
                selected_serials.insert(options.deviceId);
                deviceFound = true;
                break;
            }
        }
        if (!deviceFound) {
            std::cerr << "Device with ID " << options.deviceId << " not found.\n";
            return 1;
        }
    }
    else {
        std::cerr << "No devices foiund.\n";
        return 1;
    }

    // Automatically start data acquisition
    if (!devices.empty() && flagPaused) {
        if (!sampler.has_value()) {
            sampler.emplace(deviceManager, std::move(devices));
            flagPaused = false;
        }
    }

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

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    webSocketThread.detach();

    return 0;
}
