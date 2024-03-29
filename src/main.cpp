// clang-format off
#include <boost/asio.hpp>
//clang-format on
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <cmake_git_version/version.hpp>
#include "style.hpp"
#include "handler.hpp"
#include "apihandler.hpp"
#include "create_training_data.hpp"
#include "saves_popup.hpp"
#include "settingspopup.hpp"

int main() {
  const std::string configpath = "config/config.json";
  nlohmann::json config = load_json_file(configpath);
  set_config(configpath, config);

  constexpr ImVec2 toolBtnSize = ImVec2(200, 100); // toolbar buttons size
  std::vector<std::string> availableLanguages =
      getAvailableLanguages(load_json<std::string>(config, ("languagepath")));
  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
          load_json<std::string>(config, "language") + ".json");

  // static constexpr std::size_t captureDataReserve = 1 << 26;
  OmniscopeDeviceManager deviceManager{};
  std::vector<std::shared_ptr<OmniscopeDevice>> devices;
  std::map<Omniscope::Id, std::array<float, 3>> colorMap;

  // auto startTimepoint = std::chrono::system_clock::now();
  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::gmtime(&now_time_t);
  double xmax_paused{0};
  bool open_settings = false;
  bool open_generate_training_data = false;
  std::set<std::string> savedFileNames; // unique and ordered filenames
  bool upload_success = false;
  static bool flagPaused = true;
  bool flagDataNotSaved = true;
  std::optional<OmniscopeSampler> sampler{};
  std::map<Omniscope::Id, std::vector<std::pair<double, double>>> captureData;

  auto rstSettings = [&]() {
      sampler.reset();
      devices.clear();
      savedFileNames.clear();
      deviceManager.clearDevices();
      captureData.clear();
      return true;
      };
 
  auto render = [&]() {
    load_settings(config);
    SetupImGuiStyle(false, 0.99f);
    auto windowSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove); 

    // ############################ Menu bar ##############################
    //  main menu
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu(appLanguage["Menu"])) {
      if (ImGui::BeginMenu(appLanguage["LanOption"])) {
        for (const auto &lang : availableLanguages) 
          if (ImGui::MenuItem(lang.c_str())) {
            config["language"] = lang;
            write_json_file(configpath, config);
            appLanguage = germanLan;
          }
          if(ImGui::MenuItem(appLanguage["English"]))
            appLanguage = englishLan;

        ImGui::EndMenu();
      }
      if (ImGui::MenuItem(appLanguage["Settings"]))
        open_settings = true;
      if (ImGui::MenuItem(appLanguage["Reset"])) {
          rstSettings();
          flagPaused = true;
      }
      if (ImGui::MenuItem(
              fmt::format("{}: {}", appLanguage["Version"]
              ,CMakeGitVersion::VersionWithGit)
                  .c_str())) {
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(appLanguage["Diagnostics"])) {
      if (ImGui::BeginMenu(appLanguage["Compression"])) {
        ImGui::MenuItem(appLanguage["Anlyz_crnt_waveform"]);
        if (ImGui::MenuItem(appLanguage["Gnrt_trning_data"]))
            open_generate_training_data = true;
        ImGui::EndMenu();
      }

      // greyed out color style
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

      ImGui::MenuItem(appLanguage["Timing-Belt"]);
      ImGui::MenuItem(appLanguage["Fuel-Delivery-Pump"]);
      ImGui::MenuItem(appLanguage["Common-Rail-Pressure"]);
      ImGui::PopStyleColor(2);
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(appLanguage["Help"])) {
      if (ImGui::MenuItem(appLanguage["HelpLink"])) 
        system(("start " + load_json<std::string>(config, "helplink")).c_str());
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    // ############################ Live Capture
    ImGui::BeginChild("Live Capture", ImVec2(-1, 620));
    if (sampler.has_value())
      if (!flagPaused)
        sampler->copyOut(captureData);

    float optimal_buttonstripe_height = toolBtnSize.y * 1.1;
    if (toolBtnSize.y < (ImGui::GetTextLineHeightWithSpacing() * 1.1))
      optimal_buttonstripe_height = ImGui::GetTextLineHeightWithSpacing() * 1.1;

    ImGui::BeginChild("Buttonstripe", ImVec2(-1, optimal_buttonstripe_height),
                      false, ImGuiWindowFlags_NoScrollbar);

    // ############################ Popup Save
    if (ImGui::BeginPopupModal("Save recorded data", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {                        
     // open_save_devices = false;
      ImGui::SetItemDefaultFocus();
      saves_popup(config, language, captureData, now, now_time_t, now_tm,
                  flagDataNotSaved, sampler); 
      ImGui::EndPopup();
    }

    // ############################ Popup Reset
    if (ImGui::BeginPopupModal("Reset?", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      ImGui::Text("The measurement was not saved!\n"
                  "Would you like to save it before deleting it?\n");
      if (ImGui::Button("Continue deletion")) {
          rstSettings();
          ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Back")) 
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(ImVec2(0, 100));
    ImGui::SetNextWindowSize(ImVec2(0, 800));
    if (flagPaused) {
      if (ImGui::BeginPopupModal("Creation of learning data set", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoMove)) {
        ImGui::SetItemDefaultFocus();
        popup_create_training_data_select(config, language, upload_success);
        ImGui::EndPopup();
      }
      
      // ######################## Buttonstripe
      // Start only if devices are available, otherwise search for devices
      if (!sampler.has_value()) {
        if (ImGui::Button(appLanguage["Dvc_search"], toolBtnSize)) {
          devices.clear();
          deviceManager.clearDevices();
          initDevices(deviceManager, devices, colorMap);
        }
        ImGui::SameLine();
      }

      if (!devices.empty()) {
        // ############################ Start Button
        if (!sampler.has_value()) {
          set_button_style_to(config, "start");
          if (ImGui::Button(
                  load_json<std::string>(language, "button", "start").c_str(),
                  toolBtnSize)) {
            sampler.emplace(deviceManager, std::move(devices));
            flagPaused = false;
            flagDataNotSaved = true;
          }
          ImGui::PopStyleColor(3);
        }
      }
      // set_button_style_to(config, "standart");
    } else {
      // ############################ Stop Button
      set_button_style_to(config, "stop");
      if (ImGui::Button(
              load_json<std::string>(language, "button", "stop").c_str(),
              toolBtnSize))
        flagPaused = true;

      ImGui::PopStyleColor(3);
    }
    if (flagPaused) {
      ImGui::SameLine();

      // Start/reset the measurement when the measurement is paused,
      // followed by a query as to whether the old data should be saved
      if (sampler.has_value()) {
        ImGui::SameLine();
        set_button_style_to(config, "start");
        if (ImGui::Button("Continue", toolBtnSize)) {
          flagPaused = false;
          flagDataNotSaved = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        set_button_style_to(config, "stop");
        if (ImGui::Button("Reset", toolBtnSize)) {
          if (flagDataNotSaved) {
            ImGui::OpenPopup("Reset?");
          } else {
              rstSettings();
              flagPaused = true;
          }
        }
        ImGui::PopStyleColor(3);
      }
      ImGui::SameLine();

      // gray out "Save" button when pop-up is open
      const bool pushStyle = ImGui::IsPopupOpen("Save recorded data");

      if (pushStyle)
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            load_json<Color>(config, "text", "color", "inactive"));

      if (ImGui::Button(appLanguage["Save"], toolBtnSize)) {
         if(sampler.has_value()) 
            ImGui::OpenPopup("Save recorded data");
         else  
          ImGui::OpenPopup(appLanguage["Save warning"], ImGuiPopupFlags_NoOpenOverExistingPopup);
      }
     warning_popup(appLanguage["Save warning"], appLanguage["No_dvc_available"]);    

      if (pushStyle)
        ImGui::PopStyleColor();

      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

      ImGui::Button(appLanguage["AnalyzeData"], toolBtnSize);
      ImGui::PopStyleColor();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));
      ImGui::SameLine();

      // ############################ Button create trainings data
      if (ImGui::Button(appLanguage["Crt_trng_data"], toolBtnSize)) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::OpenPopup("Creation of learning data set");
      }
      ImGui::PopStyleColor();
    } else {
      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
      ImGui::Button("save", toolBtnSize);
      ImGui::SameLine();
      ImGui::Button(appLanguage["AnalyzeData"], toolBtnSize);
      ImGui::SameLine();
      ImGui::Button(appLanguage["Crt_trng_data"], toolBtnSize);
      ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    // ############################ Settings Menu
    std::string settingstitle =
        load_json<std::string>(language, "settings", "title");
    if (open_settings == true) {
      ImGui::OpenPopup(settingstitle.c_str());
      open_settings = false;
    }
    if (ImGui::BeginPopupModal(settingstitle.c_str(), nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      popup_settings(config, language, configpath);
      ImGui::EndPopup();
    }

    // Generate training data Menu
    if (open_generate_training_data) 
      generateTrainingData(open_generate_training_data, sampler, savedFileNames);

    // ############################ addPlots("Recording the data", ...)
    addPlots("Recording the data", captureData, flagPaused,
        [&xmax_paused](double x_max) {
            if (!flagPaused) {
                ImPlot::SetupAxes("x [Data points]", "y [ADC Value]",
                    ImPlotAxisFlags_AutoFit,
                    ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 7500, x_max + 7500,
                    ImGuiCond_Always);
            }
            else {
                xmax_paused = x_max;
                ImPlot::SetupAxes("x [Seconds]", "y [Volts]");
                ImPlot::SetupAxesLimits(0, 10, -10, 200);
                ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22, nullptr, true);
            }
        }, colorMap);

    ImGui::EndChild();

    // ############################ Devicelist
    ImGui::BeginChild("Devicelist", ImVec2(-1, 0));
    // ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    // ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
    //                       ImVec2(0.5f, 0.5f));
    ImGui::Text("devices found:");
    if (ImGui::BeginListBox("##deviceListBox", ImVec2(1024, -1))) {
        devicesList(colorMap, sampler, devices);
        ImGui::EndListBox();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(7);
    ImGui::End();
  };

  ImGuiInstance window{1920, 1080,
                       fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                   CMakeGitVersion::Project::Version)};
  while (window.run(render)) { }
  return 0;
}
