
#include "style.hpp"
#include <cmake_git_version/version.hpp>
#include "jasonhandler.hpp"
#include "websocket_client.hpp"

// For DEBUG
#include <fmt/core.h>

// Declaration of the functions
nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& captureData);

// Conversion function captureDate to JSON
nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>& data) {
    nlohmann::json jsonData = nlohmann::json::array();

    for (const auto& entry : data) {
        const auto& value = entry.second;
        for (const auto& pair : value) {
            nlohmann::json dataPoint = {{"x", pair.first}, {"y", pair.second}};
            jsonData.push_back(dataPoint);
        }
    }
    return jsonData;
}

int main() {
    
  const std::string configpath = "config/config.json";
  set_config(configpath);
  nlohmann::json config = load_json_file(configpath);
  set_json(config);
  bool flagPaused{true};
  bool flagInitState{true};

  WebSocketClient wsClient("ws://localhost:8080/");

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
      auto jsonData = captureDataToJson(captureData);
      wsClient.send(jsonData);
      //fmt::print("CaptureData: {}\n", jsonData);
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
  return 0;
}
