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
    
    CLI::App app{"OmniView"};

    app.add_option("-w, --wsuri", wsURI, "WebSocket URI")->type_name("");
    app.add_option("-d, --device", deviceId, "Omniscope Device ID")->required();

    CLI11_PARSE(app, argc, argv);
    
    // Signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    WebSocketHandler wsHandler(wsURI);

    // Initialize devices;
    devices.clear();
    deviceManager.clearDevices();
    initDevices();

    // Check if the specified device is available
    if (!devices.empty()) {
        bool deviceFound = false;
        for (const auto& device : devices) {
            if (device->getId()->serial == deviceId) {
                selected_serials.insert(deviceId);
                deviceFound = true;
                break;
            }
        }
        if (!deviceFound) {
            std::cerr << "Device with ID " << deviceId << " not found.\n";
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

    //std::this_thread::sleep_for(std::chrono::seconds(1));

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
