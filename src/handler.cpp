#include "handler.hpp"
#include "get_from_github.hpp"
#include <iostream>
#include <thread>

void initDevices() {
  constexpr int VID = 0x2e8au;
  constexpr int PID = 0x000au;

  devices = deviceManager.getDevices(VID, PID);
  std::cout << "Found " << devices.size() << " devices.\n";

  for (auto &device : devices) {
    auto id = device->getId().value();
    std::cout << "Device ID: " << id.serial << "\n";

    if (!colorMap.contains(id)) {
      // ImPlot::PushColormap(ImPlotColormap_Dark);
      // auto c = ImPlot::GetColormapColor((colorMap.size() % 7) + 1);
      colorMap[id] = std::array<float, 3>{1.0f, 1.0f, 0.0f};
      // ImPlot::PopColormap();
    }
    auto &color = colorMap[id];
    std::cout << "Setting co.or for device " << id.serial << "\n";
    device->send(Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                                   static_cast<std::uint8_t>(color[1] * 255),
                                   static_cast<std::uint8_t>(color[2] * 255)});
  }
  std::cout << "Device initialization complete.\n";
}

void set_config(const std::string &configpath) {
  if (fs::exists(configpath))
    fmt::print("found config.json\n\r");
  else {
    fmt::print("Did not find config.json.\n Download from Github\n\r");
    update_config_from_github();
  }
}

void set_json(nlohmann::json &config) {
  if (fs::exists(load_json<std::string>(config, ("languagepath"))))
    fmt::print("Found language: {}\n\r", appLanguage[Key::German]);
  else {
    fmt::print("Did not find {}.\n Download from Github\n\r",
               appLanguage[Key::German]);
    update_language_from_github();
  }
}

void consoleHandler(bool &flagInitState, nlohmann::json &config, bool &flagPaused, std::set<std::string>& selected_serials) {
    std::string input;
    while (true) {
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
            }
            else {
                std::cout << "No devices fpound.\n";
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
