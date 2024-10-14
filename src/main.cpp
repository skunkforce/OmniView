#ifndef ImDrawIdx
using ImDrawIdx = unsigned int; 
#endif
#include "popups.hpp"
#include "settingspopup.hpp"
#include "style.hpp"
#include <cmake_git_version/version.hpp>
#include "analyze_data.hpp"

int main() {
  const std::string configpath = "config/config.json";
  set_config(configpath);
  nlohmann::json config = load_json_file(configpath);
  set_json(config);
  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
                     load_json<std::string>(config, "language") + ".json");
  // local variables
  bool flagPaused{true}, development{false}, open_generate_training_data{false},open_analyze_menu{false},
      open_settings{false};
  std::once_flag configFlag;
  auto loadedFiles = captureData;
  std::map<Omniscope::Id, std::string> loadedFilenames;

  // temporary solution 
  bool LOADANALYSISDATA{false}; 
  fs::path AnalyzedFilePath(""); 

  // main loop
  auto render = [&]() {
    std::call_once(configFlag, set_inital_config, std::ref(config));
    SetupImGuiStyle(false, 0.99f);
    ImGui::SetNextWindowPos({0.f, 0.f});
    auto windowSize{ImGui::GetIO().DisplaySize};
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);

    if (development && ImGui::Button("Development"))
      ImGui::OpenPopup("Development Colors");

    // Popup-Window content
    if (ImGui::BeginPopup("Development Colors")) {
      PopupStyleEditor();
      ImGui::EndPopup();
    }

    ImGui::BeginChild("Left Side", {windowSize.x * .18f, 0.f});
    // ############################################# Side Menu
    set_side_menu(config, open_settings, open_generate_training_data,open_analyze_menu,
                  loadedFiles, loadedFilenames, LOADANALYSISDATA);
    // there're four "BeginChild"s, one as the left side
    // and three on the right side
    ImGui::EndChild(); // end child "Left Side"
    ImGui::SameLine();
    ImGui::BeginChild("Right Side", {0.f, 0.f});
    if (sampler.has_value() && !flagPaused)
      sampler->copyOut(captureData);

    // ######################################### Toolbar
    set_toolbar(config, language, flagPaused, loadedFiles, LOADANALYSISDATA);

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

    // Generate analyze data popup
    if (open_analyze_menu)
      AnalyzedFilePath = generate_analyze_menu(open_analyze_menu, LOADANALYSISDATA, captureData);

    // ############################ addPlots("Recording the data", ...)
    ImGui::Dummy({0.f, windowSize.y * .01f});
    PushPlotRegionColors();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, windowSize.x * .009f);
    ImGui::BeginChild("Record Data", {0.f, windowSize.y * 0.62f},
                      ImGuiChildFlags_Border);
    // Axes 1 to 3
    // Check if time base for axes are same
    // check if egu and timescale for plot are same
    // error if third device is added
    addPlots(
        appLanguage[Key::Recording_Data],AnalyzedFilePath,LOADANALYSISDATA,
        [flagPaused](double x_max, std::string yLabel, ImAxis_ axis,
                     double yMin, double yMax) {
          ImPlot::SetupLegend(ImPlotLocation_NorthEast);
          // auto auxFlagsMeasuring =
          //     ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoGridLines;
          // auto auxFlagsPaused = ImPlotAxisFlags_NoGridLines;
          ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22, nullptr, true);

          if (!flagPaused) {
            ImPlot::SetupAxis(axis, yLabel.c_str(), ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_X1, appLanguage[Key::Time_sec],
                              ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxisLimits(axis, yMin - 2, yMax + 2, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 1, x_max + 9,
                                    ImGuiCond_Always);

          } else {
            ImPlot::SetupAxis(ImAxis_X1, appLanguage[Key::Time_sec]);
            ImPlot::SetupAxis(axis, yLabel.c_str());
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10);
            ImPlot::SetupAxisLimits(axis, yMin - 2, yMax + 2);
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
    devicesList(flagPaused);
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
          AnalyzedFilePath = ""; 
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
