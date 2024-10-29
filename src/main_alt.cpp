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

    // Search for devices
    if (options.searchDevice) {
        searchDevices();
        return 0;
    }

    // Search for DLLs
    if (!options.dllPath.empty() && options.dllName.empty()) {
        searchDlls(options.dllPath);
        return 0;
    }

    // Test WebSocket connection
    WebSocketHandler wsHandler(options.wsURI);
    std::cout << "WebSocket connection established successfully at " << options.wsURI << std::endl;

    // If no other options such as device selection (-a, -d) are set, exit
    if (!options.all && options.deviceIds.empty() && options.dllPath.empty()) {
            return 0;
    }

    // Combine and send path of the DLL via the WebSocket
    if (!options.dllPath.empty() && !options.dllName.empty()) {
        std::string fullDllPath = createFullDllPath(options.dllPath, options.dllName);
        wsHandler.sendDllPath(fullDllPath);
        return 0;
    }


    // WebSocket case: initialize WebSocket if the URI is provided
    if (!options.wsURI.empty()) {

        // Initialize devices
        initializeDevices();
        
        // Select the devices using the CommandLine options
        if (!selectDevices(options, selected_serials)) {
            std::cerr << "No devices selected.\n";
            return 1;
        }

        // Automatically start data acquisition if devices are present
        if (!devices.empty()) {
            if (!sampler.has_value()) {
                sampler.emplace(deviceManager, std::move(devices));
            }
        }
        else {
            std::cerr << "No devices available to acquire data from.\n";
            return 1;
        }

        // Start the WebSocket handler thread to send device data
        wsHandler.startWebSocketThreadForDevices(selected_serials);

        // Main loop for WebSocket communication
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    else {
        std::cerr << "No WebSocket URI or DLL path provided, exiting.\n";
        return 1;
    }
    return 1;
}

