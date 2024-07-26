#include "handler.hpp"
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
  // std::cout << "Device initialization complete.\n";
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

void consoleHandler(bool &flagPaused, std::set<std::string>& selected_serials) {
    std::string input;
    while (running) {
        std::cout << "Enter command: ";
        std::getline(std::cin, input);
        if (input == "Search") {
            devices.clear();
            deviceManager.clearDevices();
            initDevices();
            if (!devices.empty()) {
                std::cout << "Enter the device number: ";
                std::getline(std::cin, input);
                selected_serials.clear();
                size_t pos = 0;
                while ((pos = input.find(',')) != std::string::npos) {
                    selected_serials.insert(input.substr(0, pos));
                    input.erase(0, pos + 1);
                }
                selected_serials.insert(input);
            } else {
                std::cout << "No devices found.\n";
            }
        }
        else if (input == "Start") {
            if (!devices.empty() && flagPaused) {
                if (!sampler.has_value()) {
                    sampler.emplace(deviceManager, std::move(devices));
                    flagPaused = false;
                }
            }
        }
        else if (input == "Stop") {
            if (!flagPaused) {
                flagPaused = true;
                for (auto &device : sampler->sampleDevices) {
                    device.first->send(Omniscope::Stop{});
                }
            }
        }
        else if (input == "Continue") {
            if (flagPaused && sampler.has_value()) {
                flagPaused = false;
                for (auto &device : sampler->sampleDevices) {
                    device.first->send(Omniscope::Start{});
                }
            }
        }
        else if (input == "Reset") {
            if (flagPaused && sampler.has_value()) {
                sampler.reset();
                devices.clear();
                deviceManager.clearDevices();
                flagPaused = true;
            }
        }
    }
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nSIGINT received, shutting down gracefully...\n";
        running = false;
        stopAllDevices();
    }
}
