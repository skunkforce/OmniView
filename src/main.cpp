#include "popups.hpp"
#include "settingspopup.hpp"
#include "style.hpp"
#include <cmake_git_version/version.hpp>
#include "analyze_data.hpp"

int main() {

  mainWindow mWindow; 
  
  setupSW(mWindow);  // Set up config and old language file 

  // temporary solution
  // TODO:: Change loadedFiles from datatype Scope to a concrete Filetype 
        auto loadedFiles = captureData;
        std::map<Omniscope::Id, std::string> loadedFilenames;

  // main loop
  auto render = [&]() {
    std::call_once(mWindow.configFlag, set_inital_config, std::ref(mWindow.config));
    SetupImGuiStyle(false, 0.99f);
    ImGui::SetNextWindowPos({0.f, 0.f});
    auto windowSize{ImGui::GetIO().DisplaySize};
    ImGui::SetNextWindowSize(windowSize);

    // #################################################### Begin MainWindow
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    
    // Dev Color menu 
      if (mWindow.development && ImGui::Button("Development"))
        ImGui::OpenPopup("Development Colors");

      // Popup-Window content
      if (ImGui::BeginPopup("Development Colors")) {
        PopupStyleEditor();
        ImGui::EndPopup();
      }

    set_side_menu(mWindow, loadedFiles, loadedFilenames); // style.cpp

    ImGui::SameLine();
    ImGui::BeginChild("Right Side", {0.f, 0.f});

    if (sampler.has_value() && !mWindow.flagPaused)
      sampler->copyOut(captureData);

    set_toolbar(mWindow, loadedFiles); // style.cpp

    generatePopUpMenus(mWindow); // style.cpp

    // ############################ addPlots("Recording the data", ...)
    ImGui::Dummy({0.f, windowSize.y * .01f});
    PushPlotRegionColors();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, windowSize.x * .005f);
    ImGui::BeginChild("Record Data", {0.f, windowSize.y * 0.64f},
                      ImGuiChildFlags_Border);
    // Axes 1 to 3
    // Check if time base for axes are same
    // check if egu and timescale for plot are same
    // error if third device is added
    addPlots(
        appLanguage[Key::Recording_Data], mWindow,
        [&mWindow](double x_max, std::string yLabel, ImAxis_ axis,
                     double yMin, double yMax) {
          ImPlot::SetupLegend(ImPlotLocation_NorthWest);
          if (!mWindow.flagPaused) {
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
    devicesList(mWindow.flagPaused);
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
          mWindow.setAnalyzePath(""); 
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
