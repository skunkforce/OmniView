// clang-format offload_settings
#include <boost/asio.hpp>
//clang-format on
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "create_training_data.hpp"
#include "get_from_github.hpp"
#include "languages.hpp"
#include "popups.hpp"
#include "settingspopup.hpp"
#include <cmake_git_version/version.hpp>
#include <fmt/chrono.h>
#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image/stb_image.h" // externe Libary aus Git

#include "LoadImages.hpp"
#include "menuscpp/Style.cpp"

// include menus
#include "menuscpp/DevicesMenu.cpp"
#include "menuscpp/SideBarMenu.cpp"

static std::vector<std::string>
getAvailableLanguages(std::string const &languageFolder) {
  std::vector<std::string> languages;
  for (auto const &entry :
       std::filesystem::directory_iterator(languageFolder)) {
    if (entry.is_regular_file()) {
      std::string extension = entry.path().extension().string();
      if (extension == ".json") {
        std::string filename = entry.path().filename().stem().string();
        languages.push_back(filename);
      }
    }
  }
  return languages;
}

static void set_button_style_to(nlohmann::json const &config,
                                std::string const &name) {
  ImGui::PushStyleColor(
      ImGuiCol_Button,
      ImVec4(load_json<Color>(config, "button", name, "normal")));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonHovered,
      ImVec4(load_json<Color>(config, "button", name, "hover")));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonActive,
      ImVec4(load_json<Color>(config, "button", name, "active")));
}

static void load_settings(nlohmann::json const &config) {
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg,
                        ImVec4(load_json<Color>(config, "menubar", "main")));
  ImGui::PushStyleColor(ImGuiCol_PopupBg,
                        ImVec4(load_json<Color>(config, "menubar", "popup")));
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(load_json<Color>(
                                           config, "text", "color", "normal")));
  ImGui::PushStyleColor(ImGuiCol_WindowBg,
                        ImVec4(load_json<Color>(config, "window", "color")));
  set_button_style_to(config, "standart");
}

// ###########################################################################
// ############# INT MAIN BEGINN #############################################
// ###########################################################################

int main() {
  nlohmann::json config;
  const std::string configpath = "config/config.json";
  if (std::filesystem::exists(configpath)) {
    fmt::print("found config.json\n\r");
  } else {
    fmt::print("Did not find config.json.\n Download from Github\n\r");
    update_config_from_github();
  }
  config = load_json_file(configpath);
  if (std::filesystem::exists(
          load_json<std::string>(config, ("languagepath")))) {
    fmt::print("found language: {}\n\r",
               load_json<std::string>(config, ("language")));
  } else {
    fmt::print("Did not find {}.\n Download from Github\n\r",
               load_json<std::string>(config, ("language")));
    update_language_from_github();
  }

  constexpr ImVec2 toolBtnSize{80, 80}; // toolbar buttons size
  std::vector<std::string> availableLanguages =
      getAvailableLanguages(load_json<std::string>(config, ("languagepath")));

  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
                     load_json<std::string>(config, "language") + ".json");

  constexpr int VID = 0x2e8au;
  constexpr int PID = 0x000au;
  // static constexpr std::size_t captureDataReserve = 1 << 26;
  OmniscopeDeviceManager deviceManager{};
  std::vector<std::shared_ptr<OmniscopeDevice>> devices;
  std::map<Omniscope::Id, std::array<float, 3>> colorMap;

  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::gmtime(&now_time_t);

  double xmax_paused{0};
  static bool open_settings = false;
  static bool open_generate_training_data = false;
  // unique and ordered filenames
  std::set<std::string> savedFileNames;
  static bool upload_success = false;
  static bool flagPaused = true;
  static bool flagDataNotSaved = true;
  static ImVec2 mainMenuBarSize;
  std::optional<OmniscopeSampler> sampler{};
  std::map<Omniscope::Id, std::vector<std::pair<double, double>>> captureData;

  auto addPlots = [&](const char *name, auto const &plots, auto axesSetup) {
    static std::set<std::string> firstRun;
    auto const plotRegion = ImGui::GetContentRegionAvail();
    if (ImPlot::BeginPlot(name, plotRegion)) {
      double x_min = std::numeric_limits<double>::max();
      double x_max = std::numeric_limits<double>::min();

      for (auto const &plot : plots)
        if (!plot.second.empty()) {
          x_min = std::min(x_min, plot.second.front().first);
          x_max = std::max(x_max, plot.second.back().first);
        }

      axesSetup(x_max);

      auto const limits = [&]() {
        if (!firstRun.contains(name)) {
          firstRun.insert(name);
          return ImPlotRect(x_min, x_max, 0, 0);
        }
        return ImPlot::GetPlotLimits();
      }();

      auto addPlot = [&](auto const &plot) {
        if (!plot.second.empty()) {
          auto const start = [&]() {
            auto p =
                std::lower_bound(plot.second.begin(), plot.second.end(),
                                 std::pair<double, double>{limits.X.Min, 0});
            if (p != plot.second.begin())
              return p - 1;
            return p;
          }();

          auto const end = [&]() {
            auto p =
                std::upper_bound(start, plot.second.end(),
                                 std::pair<double, double>{limits.X.Max, 0});
            if (p != plot.second.end())
              return p + 1;
            return p;
          }();
          std::size_t const stride = [&]() -> std::size_t {
            auto const s = std::distance(start, end) / (plotRegion.x * 2.0);
            if (1 >= s)
              return 1;
            return static_cast<std::size_t>(s);
          }();

          ImPlot::PlotLine(
              fmt::format("{}-{}", plot.first.type, plot.first.serial).c_str(),
              std::addressof(start->first), std::addressof(start->second),
              static_cast<std::size_t>(std::distance(start, end)) / stride, 0,
              0, 2 * sizeof(double) * stride);
        }
      };

      for (auto const &plot : plots) {
        ImPlot::SetNextLineStyle(ImVec4{colorMap[plot.first][0],
                                        colorMap[plot.first][1],
                                        colorMap[plot.first][2], 1.0f});
        addPlot(plot);
      }

      ImPlot::EndPlot();
    }
  };
  auto render = [&]() {
    load_settings(config);
    Style::SetupImGuiStyle(false, 0.99f);
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    auto windowSize{ImGui::GetIO().DisplaySize};
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    // ############################ Menu bar ##############################
    //  main menu

    // CREATE A SIDEBARMENU

    SideBarRegion::SetSideBarMenu(
        language, availableLanguages, config, configpath, open_settings,
        sampler, devices, deviceManager, captureData, flagPaused,
        open_generate_training_data, mainMenuBarSize, colorMap);

    // ############################ Live Capture
    ImGui::SetCursorPos({windowSize.x * 0.18f, windowSize.y * 0.06f});
    ImGui::BeginChild("Live Capture",
                      {windowSize.x * 0.9f, windowSize.y * 0.68f});
    if (sampler.has_value())
      if (!flagPaused)
        sampler->copyOut(captureData);

    ImGui::SetCursorPos({windowSize.x * 0.65f, windowSize.y * 0.05f});
    ImGui::BeginChild("Buttonstripe", {-1, toolBtnSize.y}, false,
                      ImGuiWindowFlags_NoScrollbar);

    // ############################ Popup Save
    if (ImGui::BeginPopupModal("Save recorded data", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      saves_popup(config, language, captureData, now, now_time_t, now_tm,
                  flagDataNotSaved);
      ImGui::EndPopup();
    }

    // ############################ Popup Reset
    if (ImGui::BeginPopupModal(appLanguage[Key::Reset_q], nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      ImGui::Text(appLanguage[Key::Measure_not_saved]);
      if (ImGui::Button(appLanguage[Key::Continue_del])) {
        sampler.reset();
        savedFileNames.clear();
        captureData.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Back]))
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos({0, 100});
    ImGui::SetNextWindowSize({0, 800});
    if (flagPaused) {
      if (ImGui::BeginPopupModal("Creation of learning data set", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoMove)) {
        ImGui::SetItemDefaultFocus();
        popup_create_training_data_select(config, language, upload_success);
        ImGui::EndPopup();
      }
      if (upload_success == true)
        ImGui::OpenPopup("upload_success");
      if (ImGui::BeginPopupModal("upload_success", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoMove)) {
        ImGui::TextUnformatted(
            load_json<std::string>(language, "training", "upload_success")
                .c_str());
        if (ImGui::Button(
                load_json<std::string>(language, "button", "ok").c_str())) {
          ImGui::CloseCurrentPopup();
          upload_success = false;
        }

        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
      }
      // ######################## Buttonstripe
      // ############################ Start Button
      if (!devices.empty()) {
        if (!sampler.has_value()) {
          set_button_style_to(config, "start");
          if (ImGui::Button(appLanguage[Key::Start], toolBtnSize)) {
            sampler.emplace(deviceManager, std::move(devices));
            flagPaused = false;
            flagDataNotSaved = true;
          }
          ImGui::PopStyleColor(3);
        }
      }
    } else {
      // ############################ Stop Button
      set_button_style_to(config, "stop");
      if (ImGui::Button(appLanguage[Key::Stop], toolBtnSize))
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
        if (ImGui::Button(appLanguage[Key::Continue], toolBtnSize)) {
          flagPaused = false;
          flagDataNotSaved = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        set_button_style_to(config, "stop");
        if (ImGui::Button(appLanguage[Key::Reset], toolBtnSize)) {
          if (flagDataNotSaved) {
            ImGui::OpenPopup(appLanguage[Key::Reset_q]);
          } else {
            sampler.reset();
            savedFileNames.clear();
            captureData.clear();
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
      if (ImGui::Button(appLanguage[Key::Save], toolBtnSize)) {
        if (sampler.has_value())
          ImGui::OpenPopup("Save recorded data");
        else
          ImGui::OpenPopup(appLanguage[Key::Save_warning],
                           ImGuiPopupFlags_NoOpenOverExistingPopup);
      }
      info_popup(appLanguage[Key::Save_warning],
                 appLanguage[Key::No_dvc_available]);

      if (pushStyle)
        ImGui::PopStyleColor();

      ImGui::SameLine();
      /* ImGui::PushStyleColor(
           ImGuiCol_Text, load_json<Color>(config, "text", "color",
       "inactive"));

       ImGui::Button(appLanguage["AnalyzeData"], toolBtnSize);
       ImGui::PopStyleColor();
       ImGui::PushStyleColor(
           ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));
       ImGui::SameLine();*/

      // ############################ Button create trainings data
      // ##############################
      /*if (ImGui::Button(appLanguage["Crt_trng_data"], toolBtnSize)) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::OpenPopup("Creation of learning data set");
      }*/

      // ImGui::PopStyleColor();

    } else {
      /*ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
      ImGui::Button("save", toolBtnSize);

      ImGui::SameLine();
      ImGui::Button(appLanguage["AnalyzeData"], toolBtnSize);
      ImGui::SameLine();
      ImGui::Button(appLanguage["Crt_trng_data"], toolBtnSize);
      ImGui::PopStyleColor();*/
    }
    ImGui::EndChild(); // child "Buttonstripe"
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
      generateTrainingData(open_generate_training_data, captureData,
                           savedFileNames, config);

    // ############################ addPlots("Recording the data", ...)
    ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.8f, 0.8f, 0.8f, 0.8f});
    ImGui::BeginChild("plot region", {windowSize.x, windowSize.y * 0.5f});

    ImGui::SetCursorPos({windowSize.x * 0.015f, windowSize.y * 0.024f});
    ImGui::BeginChild("record data",
                      {windowSize.x * 0.78f, windowSize.y * 0.45f});

    SetMainWindowStyle();
    addPlots("Recording the data", captureData, [&xmax_paused](double x_max) {
      if (!flagPaused) {
        ImPlot::SetupAxes("x [Data points]", "y [ADC Value]",
                          ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 7500, x_max + 7500,
                                ImGuiCond_Always);
      } else {
        xmax_paused = x_max;
        ImPlot::SetupAxes("x [Seconds]", "y [Volts]");
        ImPlot::SetupAxesLimits(0, 10, -10, 200);
        ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22, nullptr, true);
      }
    });

    ImGui::EndChild(); // child "record data"
    ImGui::PopStyleColor();
    ImGui::EndChild(); // child "plot region"
    ImGui::EndChild(); // child "Live Capture" 

    // ############################ Devicelist
    Style::SetupImGuiStyle(false, 0.99f);

    // Create Devices Menu at the bottom of the programm
    DevicesRegion::SetDevicesMenu(colorMap, sampler, devices);
    ImGui::SameLine();
    ImGui::End();
    ImGui::PopStyleColor(7);
  };

  ImGuiInstance window{1280, 760,
                       fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                   CMakeGitVersion::Project::Version)};
  while (window.run(render)) {
  }
  return 0;
}
