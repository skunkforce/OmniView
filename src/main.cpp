#include "websockethandler.hpp"
#include "handler.hpp"
#include "dllhandler.hpp"
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
    if (!options.dllSearchPath.empty() && options.dllName.empty()) {
        DllHandler::searchDlls(options.dllSearchPath);
        return 0;
    }

    // Start DLL data transfer if the DLL path is provided
    if (!options.dllPath.empty()) {
        DllHandler::startDllDataTransfer(options.dllPath);
        return 0;
    }
    
    // WebSocket case: initialize WebSocket if the URI is provided
    if (!options.wsURI.empty()) {

        // Test WebSocket connection
        WebSocketHandler wsHandler(options.wsURI);
        std::cout << "WebSocket connection established successfully at " << options.wsURI << std::endl;

        // If no other options such as device selection (-a, -d) are set, exit
        if (!options.all && options.deviceIds.empty() && options.dllSearchPath.empty()) {
            return 0;
        }

        // If DLL path and DLL name have been specified
        if (!options.dllSearchPath.empty() && !options.dllName.empty()) {
            std::string dllFullPath = options.dllSearchPath + "/" + options.dllName;
            DllHandler::startDllDataTransfer(dllFullPath);
            return 0;
        }

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
        wsHandler.startWebSocketThread(selected_serials);

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

