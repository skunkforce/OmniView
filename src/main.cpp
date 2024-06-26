#include "popups.hpp"
#include "settingspopup.hpp"
#include "style.hpp"
#include <cmake_git_version/version.hpp>

int main() {
  const std::string configpath = "config/config.json";
  set_config(configpath);
  nlohmann::json config = load_json_file(configpath);
  set_json(config);
  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
                     load_json<std::string>(config, "language") + ".json");
  // local variables
  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::gmtime(&now_time_t);
  bool flagPaused{true};
  bool Development{false}, flagInitState{true},
      open_generate_training_data{false}, open_settings{false};
  auto loadedFiles = captureData;
  std::map<Omniscope::Id, std::string> loadedFilenames;

  // main loop
  auto render = [&]() {
    if (flagInitState) {
      set_inital_config(config);
      flagInitState = false;
    }
    SetupImGuiStyle(false, 0.99f);
    ImGui::SetNextWindowPos({0.f, 0.f});
    auto windowSize{ImGui::GetIO().DisplaySize};
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);

    if (Development && ImGui::Button("Development"))
      ImGui::OpenPopup("Development Colors");

    // Popup-Window content
    if (ImGui::BeginPopup("Development Colors")) {
      PopupStyleEditor();
      ImGui::EndPopup();
    }

    ImGui::BeginChild("Left Side", {windowSize.x * .18f, 0.f});
    // ############################################# Side Menu
    set_side_menu(config, open_settings, open_generate_training_data,
                  loadedFiles, loadedFilenames);
    // there're four "BeginChild"s, one as the left side
    // and three on the right side
    ImGui::EndChild(); // end child "Left Side"
    ImGui::SameLine();
    ImGui::BeginChild("Right Side", {0.f, 0.f});
    if (sampler.has_value() && !flagPaused)
      sampler->copyOut(captureData);

    // ######################################### Toolbar
    set_toolbar(config, language, flagPaused, loadedFiles);

    // ############################ Settings Menu
    static int title = 0;
    static std::vector<std::string> titles(2); // two languages
    if (open_settings) {
      const auto EngItr = englishLan.find(Key::Settings);
      const auto GrmItr = germanLan.find(Key::Settings);
      // check returned value from find() and set titles
      if (EngItr != englishLan.end() && GrmItr != germanLan.end()) {
        titles[0] = (std::string)EngItr->second + "###ID";
        titles[1] = (std::string)GrmItr->second + "###ID";
        ImGui::OpenPopup(titles[title].c_str());
      } else
        fmt::println("Settings values not found.");
      open_settings = false;
    }
    if (ImGui::BeginPopupModal(titles[title].c_str(), nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      popup_settings(config, configpath, title);
      ImGui::EndPopup();
    }
    // Generate training data popup
    if (open_generate_training_data)
      generateTrainingData(open_generate_training_data, captureData,
                           savedFileNames);

    // ############################ addPlots("Recording the data", ...)
    ImGui::Dummy({0.f, windowSize.y * .01f});
    PushPlotRegionColors();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, windowSize.x * .009f);
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
    ImGui::PopStyleVar();
    PopPlotRegionColors();
    // ############################ Devicelist
    SetDeviceMenuStyle();
    ImGui::Dummy({0.f, windowSize.y * .01f}); 
    ImGui::BeginChild("Devicelist");
    ImGui::Dummy({windowSize.x * .36f, 0.f});
    ImGui::SameLine();
    ImGui::Text(appLanguage[Key::Devices_found]);
    devicesList();
    if (!loadedFiles.empty()) { // if devices were successfully loaded from file
      static std::map<Omniscope::Id, BoolWrapper> loadedFilesChkBxs;
      for (auto it = loadedFiles.begin(); it != loadedFiles.end();) {
        ImGui::PushID(&it->first); // make unique IDs
        if (ImGui::Checkbox("##", &loadedFilesChkBxs[it->first].b))
          if (loadedFilesChkBxs[it->first].b) { // if checked
            if (!captureData.contains(it->first)) {
              captureData.emplace(it->first, it->second);
              // set different colors for files data
              if (!colorMap.contains(it->first)) {
                ImPlot::PushColormap(ImPlotColormap_Dark);
                auto color =
                    ImPlot::GetColormapColor((colorMap.size() % 7) + 1);
                colorMap[it->first] = {color.x, color.y, color.z};
                ImPlot::PopColormap();
              }
            }
          } else
            captureData.erase(it->first);
        ImGui::SameLine();
        ImGui::TextUnformatted(loadedFilenames[it->first].c_str());
        ImGui::SameLine();
        if (ImGui::Button(appLanguage[Key::Reset])) {
          captureData.erase(it->first);
          loadedFilenames.erase(it->first);
          loadedFilesChkBxs[it->first].b = false;
          it = loadedFiles.erase(it);
        } else 
          it++;
        ImGui::PopID();
      } // end of for-loop
    } 
    ImGui::EndChild(); // end child "Devicelist"
    ImGui::EndChild(); // end child "Right Side"
    ImGui::End();
  };
  ImGuiInstance window{1500, 800,
                       fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                   CMakeGitVersion::Project::Version)};
  while (window.run(render))
    ;
  return 0;
}