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
  static bool flagPaused{true};
  bool flagDataNotSaved{true}, Development{false}, flagInitState{true},
      loadedFileChkBx{false}, open_generate_training_data{false},
      open_settings{false};
  dvcPair loadedDvc;

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
    const ImVec2 toolBtnSize{windowSize.x * .1f,
                             windowSize.y * .1f}; // toolbar buttons size
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
    static fs::path loadedFileName;
    set_side_menu(config, open_settings, open_generate_training_data,
                  loadedFileName, loadedDvc);
    // there're four "BeginChild"s, one as the left side
    // and three on the right side
    ImGui::EndChild(); // end child "Left Side"
    ImGui::SameLine();
    ImGui::BeginChild("Right Side", {0.f, 0.f});
    if (sampler.has_value() && !flagPaused)
      sampler->copyOut(captureData);
    ImGui::BeginChild("Buttonstripe", {-1.f, windowSize.y * .1f}, false,
                      ImGuiWindowFlags_NoScrollbar);
    // ############################ Popup Save
    if (ImGui::BeginPopupModal(appLanguage[Key::Save_Recorded_Data], nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      saves_popup(config, language, now, now_time_t, now_tm, flagDataNotSaved);
      ImGui::EndPopup();
    }
    // ############################ Popup Reset
    if (ImGui::BeginPopupModal(appLanguage[Key::Reset_q], nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      ImGui::Text(appLanguage[Key::Measure_not_saved]);
      if (ImGui::Button(appLanguage[Key::Continue_del])) {
        rstSettings(loadedDvc);
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Back]))
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    if (flagPaused) {
      // ######################## Buttonstripe
      if (!devices.empty())
        if (!sampler.has_value()) {
          set_button_style_to(config, "start"); // Start Button
          if (ImGui::Button(appLanguage[Key::Start], toolBtnSize)) {
            sampler.emplace(deviceManager, std::move(devices));
            flagPaused = false;
            flagDataNotSaved = true;
          }
          ImGui::PopStyleColor(3);
        }
      // set_button_style_to(config, "standart");
    } else {
      // ############################ Stop Button
      set_button_style_to(config, "stop");
      if (ImGui::Button(appLanguage[Key::Stop], toolBtnSize))
        flagPaused = true;
      ImGui::PopStyleColor(3);
    }
    if (flagPaused) {
      // Start/reset the measurement when the measurement is paused,
      // followed by a query as to whether the old data should be saved
      if (sampler.has_value()) {
        ImGui::SameLine();
        set_button_style_to(config, "start");
        if (ImGui::Button(appLanguage[Key::Continue], toolBtnSize)) {
          flagPaused = false;
          flagDataNotSaved = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        set_button_style_to(config, "stop");
        if (ImGui::Button(appLanguage[Key::Reset], toolBtnSize)) {
          if (flagDataNotSaved)
            ImGui::OpenPopup(appLanguage[Key::Reset_q]);
          else {
            rstSettings(loadedDvc);
            flagPaused = true;
          }
        }
        ImGui::PopStyleColor(3);
      }
      ImGui::SameLine();

      // gray out "Save" button when pop-up is open
      const bool pushStyle =
          ImGui::IsPopupOpen(appLanguage[Key::Save_Recorded_Data]);

      if (pushStyle)
        ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
      if (ImGui::Button(appLanguage[Key::Save], toolBtnSize)) {
        if (sampler.has_value())
          ImGui::OpenPopup(appLanguage[Key::Save_Recorded_Data]);
        else
          ImGui::OpenPopup(appLanguage[Key::Save_warning],
                           ImGuiPopupFlags_NoOpenOverExistingPopup);
      }
      info_popup(appLanguage[Key::Save_warning],
                 appLanguage[Key::No_dvc_available]);

      if (pushStyle)
        ImGui::PopStyleColor();
    } else {
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
      ImGui::Button(appLanguage[Key::Save], toolBtnSize);
      ImGui::PopStyleColor();
    }
    ImGui::EndChild(); // end child "Buttonstripe"
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

    addPlots("Recording the data", [](double x_max) {
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
    if (loadedDvc.second.size()) { // if device was successfully loaded from file
      if (ImGui::Checkbox("##", &loadedFileChkBx))
        if (loadedFileChkBx)
          captureData[loadedDvc.first] = loadedDvc.second;
        else
          fmt::println("{} device erased from list.",
                       captureData.erase(loadedDvc.first));
      ImGui::SameLine();
      ImGui::TextUnformatted(loadedFileName.filename().string().c_str());
      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Reset])) {
        captureData.erase(loadedDvc.first);
        loadedDvc = {};
        loadedFileChkBx = false;
      }
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