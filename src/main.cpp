// #include "OmniscopeCommunication.hpp"
// clang-format off
#include <boost/asio.hpp>
//clang-format on
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "apihandler.hpp"
#include "create_training_data.hpp"
#include "get_from_github.hpp"
#include "saves_popup.hpp"
#include "jasonhandler.hpp"
#include "settingspopup.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <algorithm>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <thread>
#include <cmake_git_version/version.hpp>
// clang-format off
#include <imfilebrowser.h>
// clang-format on

namespace ImGui {
inline void SetupImGuiStyle(bool bStyleDark_, float alpha_) {
  ImGuiStyle &style = ImGui::GetStyle();

  // light style from Pacôme Danhiez (user itamago)
  // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
  // style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f,
  // 0.00f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f,
  // 0.39f, 1.00f); style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f,
  // 0.59f, 0.98f, 0.78f); style.Colors[ImGuiCol_ColumnActive]          =
  // ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  // style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f,
  // 0.50f); style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f,
  // 0.36f, 1.00f); style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f,
  // 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  // style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f,
  // 0.35f);

  if (bStyleDark_) {
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      float H, S, V;
      ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

      if (S < 0.1f) {
        V = 1.0f - V;
      }
      ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
      if (col.w < 1.00f) {
        col.w *= alpha_;
      }
    }
  } else {
    // std::cout << ImGuiCol_COUNT << std::endl;
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      if (col.w < 1.00f) {
        col.x *= alpha_;
        col.y *= alpha_;
        col.z *= alpha_;
        col.w *= alpha_;
      }
    }
  }
}
} // namespace ImGui

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

  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = load_json<float>(config, "text", "scale");
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

  std::vector<std::string> availableLanguages =
      getAvailableLanguages(load_json<std::string>(config, ("languagepath")));

  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
                     load_json<std::string>(config, "language") + ".json");

  static constexpr int VID = 0x2e8au;
  static constexpr int PID = 0x000au;
  // static constexpr std::size_t captureDataReserve = 1 << 26;
  OmniscopeDeviceManager deviceManager{};
  std::vector<std::shared_ptr<OmniscopeDevice>>
      devices; // = deviceManager.getDevices(VID, PID);
  // auto newDevices = devices;
  std::map<Omniscope::Id, std::array<float, 3>> colorMap;
  auto initDevices = [&]() {
    devices = deviceManager.getDevices(VID, PID);

    for (auto &device : devices) {
      auto id = device->getId().value();
      if (!colorMap.contains(id)) {
        ImPlot::PushColormap(ImPlotColormap_Dark);
        auto c = ImPlot::GetColormapColor((colorMap.size() % 7) + 1);
        colorMap[id] = std::array<float, 3>{c.x, c.y, c.z};
        ImPlot::PopColormap();
      }

      auto &color = colorMap[id];
      device->send(
          Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                            static_cast<std::uint8_t>(color[1] * 255),
                            static_cast<std::uint8_t>(color[2] * 255)});
    }
  };

  //   auto startTimepoint = std::chrono::system_clock::now();
  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::gmtime(&now_time_t);

  double xmax_paused{0};
  static bool open_settings = false;
  static bool upload_success = false;
  static bool flagPaused = true;
  static bool flagDataNotSaved = true;
  static ImVec2 mainMenuBarSize;
  std::optional<OmniscopeSampler> sampler{};
  std::map<Omniscope::Id, std::vector<std::pair<double, double>>> captureData;

  std::string path;
  path.resize(256);

  auto addPlots = [&, firstRun = std::set<std::string>{}](
                      auto const &name, auto const &plots,
                      auto axesSetup) mutable {
    auto const plotRegion = ImGui::GetContentRegionAvail();
    if (ImPlot::BeginPlot(name, plotRegion)) {
      double x_min = std::numeric_limits<double>::max();
      double x_max = std::numeric_limits<double>::min();

      for (auto const &plot : plots) {
        if (!plot.second.empty()) {
          x_min = std::min(x_min, plot.second.front().first);
          x_max = std::max(x_max, plot.second.back().first);
        }
      }

      axesSetup(x_min, x_max);

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
            if (p != plot.second.begin()) {
              return p - 1;
            }
            return p;
          }();

          auto const end = [&]() {
            auto p =
                std::upper_bound(start, plot.second.end(),
                                 std::pair<double, double>{limits.X.Max, 0});
            if (p != plot.second.end()) {
              return p + 1;
            }
            return p;
          }();
          std::size_t const stride = [&]() -> std::size_t {
            auto const s = std::distance(start, end) / (plotRegion.x * 2.0);
            if (1 >= s) {
              return 1;
            }
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
    ImGui::SetupImGuiStyle(false, 0.99f);
    ImGui::SetNextWindowPos(ImVec2(0.0f, mainMenuBarSize.y));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove); //

    // ############################ Menu bar ##############################
    //  main menu
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu(
            load_json<std::string>(language, "menubar", "menu", "label")
                .c_str())) {
      if (ImGui::BeginMenu(load_json<std::string>(language, "menubar", "menu",
                                                  "language_option")
                               .c_str())) {
        for (const auto &lang : availableLanguages) {
          if (ImGui::MenuItem(lang.c_str())) {
            config["language"] = lang;
            write_json_file(configpath, config);
          }
        }

        ImGui::EndMenu();
      }
      if (ImGui::MenuItem(
              load_json<std::string>(language, "menubar", "menu", "settings")
                  .c_str())) {
        open_settings = true;
      }

      if (ImGui::MenuItem(
              load_json<std::string>(language, "menubar", "menu", "reset")
                  .c_str())) {
        sampler.reset();
        devices.clear();
        deviceManager.clearDevices();
        captureData.clear();
        flagPaused = true;
      }

      if (ImGui::MenuItem(
              fmt::format("Version: {}", CMakeGitVersion::VersionWithGit)
                  .c_str())) {
      }
      ImGui::EndMenu();
    }
    /*
    if (ImGui::BeginMenu(
            load_json<std::string>(language, "menubar", "view", "label")
                .c_str())) {
      ImGui::EndMenu();
    }*/

    if (ImGui::BeginMenu(
            load_json<std::string>(language, "menubar", "help", "label")
                .c_str())) {
      if (ImGui::MenuItem(
              load_json<std::string>(language, "helplink").c_str())) {
        system(("start " + load_json<std::string>(config, "helplink")).c_str());
      }

      ImGui::EndMenu();
    }

    mainMenuBarSize = ImGui::GetItemRectSize();
    ImGui::EndMainMenuBar();

    // ############################ Live Capture
    // ##############################
    ImGui::BeginChild("Live Capture", ImVec2(-1, 620));
    if (sampler.has_value()) {
      if (!flagPaused) {
        sampler->copyOut(captureData);
      }
    }

    addPlots("Aufnahme der Daten", captureData,
             [&sampler, &xmax_paused](auto /*x_min*/, auto x_max) {
               if (!flagPaused) {
                 ImPlot::SetupAxes("x [Datenpunkte]", "y [ADC Wert]",
                                   ImPlotAxisFlags_AutoFit,
                                   ImPlotAxisFlags_AutoFit);
                 ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 7500, x_max + 7500,
                                         ImGuiCond_Always);
               } else {
                 xmax_paused = x_max;
                 ImPlot::SetupAxes("x [Datenpunkte]", "y [ADC Wert]", 0, 0);
               }
             });

    ImGui::EndChild();
    float optimal_buttonstripe_height;
    if (load_json<float>(config, "button", "sizey") <
        (ImGui::GetTextLineHeightWithSpacing() * 1.1)) {
      optimal_buttonstripe_height =
          (ImGui::GetTextLineHeightWithSpacing() * 1.1);
    } else {
      optimal_buttonstripe_height =
          load_json<float>(config, "button", "sizey") * 1.1;
    }
    ImGui::BeginChild("Buttonstripe", ImVec2(-1, optimal_buttonstripe_height),
                      false, ImGuiWindowFlags_NoScrollbar);

    // ############################ Popup Speichern
    // ##############################
    if (ImGui::BeginPopupModal("Speichern der aufgenommenen Daten", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      saves_popup(config, language, captureData, now, now_time_t, now_tm, path,
                  flagDataNotSaved);

      ImGui::SameLine();
      if (ImGui::Button(
              load_json<std::string>(language, "button", "back").c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }
    // ############################ Popup Zurücksetzen
    // ##############################
    if (ImGui::BeginPopupModal("Zurücksetzen?", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      ImGui::Text("Die Messung wurde nicht gespeichert!\n"
                  "Möchten Sie diese vor dem Löschen speichern?\n");
      if (ImGui::Button("Löschen fortsetzen",
                        ImVec2(load_json<Size>(config, "button")))) {
        sampler.reset();
        devices.clear();
        deviceManager.clearDevices();
        captureData.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Zurück", ImVec2(load_json<Size>(config, "button")))) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(ImVec2(0, 100));
    ImGui::SetNextWindowSize(ImVec2(0, 800));
    if (flagPaused) {
      if (ImGui::BeginPopupModal("Erstellung Lerndatensatz", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoMove)) {
        ImGui::SetItemDefaultFocus();

        popup_create_training_data_select(config, language, upload_success);
        ImGui::EndPopup();
      }
      if (upload_success == true) {
        ImGui::OpenPopup("upload_success");
      }
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
      // ################################
      // Start nur wenn Devices vorhanden sind, sonst Suche Geräte
      if (!sampler.has_value()) {
        if (ImGui::Button("Suche Geräte",
                          ImVec2(load_json<Size>(config, "button")))) {
          devices.clear();
          deviceManager.clearDevices();
          initDevices();
        }
        ImGui::SameLine();
      }

      if (!devices.empty()) {
        // ############################ Start Button
        // ##############################
        if (!sampler.has_value()) {
          set_button_style_to(config, "start");
          if (ImGui::Button(
                  load_json<std::string>(language, "button", "start").c_str(),
                  ImVec2(load_json<Size>(config, "button")))) {
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
      // ##############################
      set_button_style_to(config, "stop");
      if (ImGui::Button(
              load_json<std::string>(language, "button", "stop").c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
        flagPaused = true;
      }
      ImGui::PopStyleColor(3);
    }
    if (flagPaused) {
      ImGui::SameLine();

      // Start / Zurücksetzen der Messung bei pausierter Messung
      // mit anschließender Abfrage ob die alten Daten gespeichert
      // werden sollen
      if (sampler.has_value()) {
        ImGui::SameLine();
        set_button_style_to(config, "start");
        if (ImGui::Button("Fortsetzen",
                          ImVec2(load_json<Size>(config, "button")))) {
          flagPaused = false;
          flagDataNotSaved = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        set_button_style_to(config, "stop");
        if (ImGui::Button("Zurücksetzen",
                          ImVec2(load_json<Size>(config, "button")))) {
          if (flagDataNotSaved) {
            ImGui::OpenPopup("Zurücksetzen?");
          } else {
            sampler.reset();
            devices.clear();
            deviceManager.clearDevices();
            captureData.clear();
            flagPaused = true;
          }
        }
        ImGui::PopStyleColor(3);
      }
      ImGui::SameLine();
      if (ImGui::Button(
              load_json<std::string>(language, "button", "save").c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
        ImGui::OpenPopup("Speichern der aufgenommenen Daten");
      }
      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
      if (ImGui::Button(
              load_json<std::string>(language, "button", "analyse_data")
                  .c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
      }
      ImGui::PopStyleColor();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));
      ImGui::SameLine();

      // ############################ Button create trainings data
      // ##############################
      if (ImGui::Button(
              load_json<std::string>(language, "button", "create_training_data")
                  .c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::OpenPopup("Erstellung Lerndatensatz");
      }
      ImGui::PopStyleColor();
    } else {
      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
      if (ImGui::Button(
              load_json<std::string>(language, "button", "save").c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
      }
      ImGui::SameLine();
      if (ImGui::Button(
              load_json<std::string>(language, "button", "analyse_data")
                  .c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
      }
      ImGui::SameLine();
      if (ImGui::Button(
              load_json<std::string>(language, "button", "create_training_data")
                  .c_str(),
              ImVec2(load_json<Size>(config, "button")))) {
      }
      ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    // ############################ Settings Menu
    // ##############################
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

    // ############################ Devicelist
    // ##############################
    ImGui::BeginChild("Devicelist", ImVec2(-1, 0));
    // ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    // ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
    //                       ImVec2(0.5f, 0.5f));

    ImGui::Text("gefundene Geräte:");
    if (ImGui::BeginListBox("##deviceListBox", ImVec2(1024, -1))) {
      auto doDevice = [&](auto &device, auto msg) {
        auto &color = colorMap[device->getId().value()];
        if (ImGui::ColorEdit3(
                fmt::format("{:<32}",
                            fmt::format("{}-{}", device->getId().value().type,
                                        device->getId().value().serial))
                    .c_str(),
                color.data(),
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker |
                    ImGuiColorEditFlags_NoTooltip)) {
          device->send(
              Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                                static_cast<std::uint8_t>(color[1] * 255),
                                static_cast<std::uint8_t>(color[2] * 255)});
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(
            fmt::format("HW: v{}.{}.{} SW: v{}.{}.{}    ",
                        device->getId().value().hwVersion.major,
                        device->getId().value().hwVersion.minor,
                        device->getId().value().hwVersion.patch,
                        device->getId().value().swVersion.major,
                        device->getId().value().swVersion.minor,
                        device->getId().value().swVersion.patch)
                .c_str());
        ImGui::SameLine();
        if (device->isRunning()) {
          ImGui::TextUnformatted(fmt::format("{}", msg).c_str());
        } else {
          ImGui::TextUnformatted(fmt::format("Fehler").c_str());
        }
      };

      if (sampler.has_value()) {
        for (auto &device : sampler->sampleDevices) {
          doDevice(device.first, "Messung");
        }
      } else {
        for (auto &device : devices) {
          doDevice(device, "Bereit");
        }
      }
      ImGui::EndListBox();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::End();
    ImGui::PopStyleColor(7);
  };

  ImGuiInstance window{1920, 1080,
        fmt::format("{} {}",
                      CMakeGitVersion::Target::Name
                    , CMakeGitVersion::Project::Version)};

  while (window.run(render)) {
  }
  return 0;
}
