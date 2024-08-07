#include "handler.hpp"
#include "websockethandler.hpp"
#include <iostream>
#include <thread>
#include <csignal>

// A list of colors
const std::array<std::array<float, 3>, 7> predefinedColors = {{
    {1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f}
}};

void initDevices() {
  constexpr int VID = 0x2e8au;
  constexpr int PID = 0x000au;

  devices = deviceManager.getDevices(VID, PID);
  std::cout << "Found " << devices.size() << " devices.\n";

  for (auto &device : devices) {
    auto id = device->getId().value();

    if (!colorMap.contains(id)) {
      size_t colorIndex = colorMap.size() % predefinedColors.size();
      colorMap[id] = predefinedColors[colorIndex];
    }
    auto &color = colorMap[id];
    device->send(Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                                   static_cast<std::uint8_t>(color[1] * 255),
                                   static_cast<std::uint8_t>(color[2] * 255)});
  }
  std::cout << "Device initialization complete.\n";
}

void searchDevices() {
    initDevices();
    if (devices.empty()) {
        std::cout << "No devices found.\n";
    }
    stopAllDevices();
}

void stopAllDevices() {
    if (sampler.has_value()) {
        for (auto &device : sampler->sampleDevices) {
            device.first->send(Omniscope::Stop{});
        }
    }
    devices.clear();
    deviceManager.clearDevices();
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nSIGINT received, shutting down gracefully...\n";
        running = false;
        stopAllDevices();
    }
}

bool selectDevices(const CommandLineOptions& options, std::set<std::string>& selected_serials) {
    if (options.all) {
        // If zhe "-a" flag is set, add all devices
        for (const auto& device : devices) {
            selected_serials.insert(device->getId()->serial);
        }
    }
    else if (!options.deviceIds.empty()) {
        // If specific IDs have been specified
        for (const auto& deviceId : options.deviceIds) {
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
                return false;
            }
        }
    }
    else {
        std::cerr << "No devices found.\n";
        return false;
    }
    return true;
}
