#include "style.hpp"
#include <cmake_git_version/version.hpp>
#include "jasonhandler.hpp"
#include "websockethandler.hpp"

// Separate function to handle console input
void consoleHandler(bool &flagInitState, nlohmann::json &config, bool &flagPaused, std::set<std::string>& selected_serials) {
    std::string input;
    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, input);
        if (input == "Search Device") {
            devices.clear();
            deviceManager.clearDevices();
            initDevices();
            if (flagInitState) {
                set_inital_config(config);
                flagInitState = false;
            }
            if (!devices.empty()) {
                std::cout << "Enter the serial number of the device to select: ";
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

int main() {
    
  const std::string configpath = "config/config.json";
  set_config(configpath);
  nlohmann::json config = load_json_file(configpath);
  set_json(config);
  bool flagPaused{true};
  bool flagInitState{true};
  std::set<std::string> selected_serials;

  WebSocketHandler wsHandler("ws://localhost:8080/");

  // Start console handler thread
  std::thread consoleThread(consoleHandler, std::ref(flagInitState), std::ref(config), std::ref(flagPaused), std::ref(selected_serials));

  // main loop
  auto render = [&]() {
    if (flagInitState) {
      set_inital_config(config);
      flagInitState = false;
    }
    ImGui::SetNextWindowPos({0.f, 0.f});
    auto windowSize{ImGui::GetIO().DisplaySize};
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    // ############################################# Side Menu
    set_side_menu(config);

    if (sampler.has_value() && !flagPaused) {
      sampler->copyOut(captureData);
      wsHandler.send(captureData, selected_serials);
    }

    // ######################################### Toolbar
    set_toolbar(config, flagPaused);
    ImGui::BeginChild("Record Data", {0.f, windowSize.y * 0.62f},
                      ImGuiChildFlags_Border);

    addPlots("Recording the data", [flagPaused](double x_max) {
      if (!flagPaused) {
        ImPlot::SetupAxes("x [Data points]", "y [ADC Value]",
                          ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 7500, x_max + 7500,
                                ImGuiCond_Always);
      } else {
        ImPlot::SetupAxes("x [Seconds]", "y [Volts]");
        ImPlot::SetupAxesLimits(0, 10, -10, 200);
        ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22, nullptr, true);
        ImPlot::SetupLegend(ImPlotLocation_NorthEast);
      }
    });
    ImGui::EndChild(); // end child Record Data

    ImGui::End();
  };
  ImGuiInstance window{1500, 800,
                       fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                   CMakeGitVersion::Project::Version)};
  while (window.run(render))
    ;

  consoleThread.join();

  return 0;
}
