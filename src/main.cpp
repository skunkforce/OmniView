#include "VCDS_data.hpp"
#include "apihandler.hpp"
#include "imagesHeaderToolbar.hpp"
#include "languages.hpp"
#include "popups.hpp"
#include "settingspopup.hpp"
#include "style.hpp"
#include <boost/asio.hpp>
#include <cmake_git_version/version.hpp>
#include <fmt/chrono.h>
#include <fmt/core.h>

int main() {
  bool Development = true;

  const std::string configpath = "config/config.json";
  set_config(configpath);
  nlohmann::json config = load_json_file(configpath);
  set_json(config);
  constexpr ImVec2 toolBtnSize{200.f, 100.f}; // toolbar buttons size
  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
                     load_json<std::string>(config, "language") + ".json");

  // local variables
  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::gmtime(&now_time_t);
  double xmax_paused{0};
  bool open_settings = false;
  bool open_generate_training_data = false;
  bool open_VCDS = false;
  static bool flagPaused = true;
  bool flagDataNotSaved = true;

  bool upload_success = false;

  // main loop
  auto render = [&]() {
    SetupImGuiStyle(false, 0.99f, config);
    ImGui::SetNextWindowPos({0.f, 0.f});
    auto windowSize{ImGui::GetIO().DisplaySize};
    ImGui::SetNextWindowSize(windowSize);

    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);

    if (Development) {

      if (ImGui::Button("Development")) {
        ImGui::OpenPopup("Development Colors");
      }

      // Popup-Fensterinhalt
      if (ImGui::BeginPopup("Development Colors")) {
        PopupStyleEditor(false, 0.99f, config);
        ImGui::EndPopup();
      }
    }

    ImGui::BeginChild("Left Side", {windowSize.x * .2f, 0.f});
    set_side_menu(config, flagPaused, open_settings,
                  open_generate_training_data, open_VCDS);
    // there're four "BeginChild"s, one as the left side and three on the right
    // side
    ImGui::EndChild(); // end child "Left Side"
    ImGui::SameLine();
    ImGui::BeginChild("Right Side", {0.f, 0.f});
    if (sampler.has_value() && !flagPaused)
      sampler->copyOut(captureData);
    ImGui::BeginChild("Buttonstripe", {-1.f, 100.f}, false,
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
        rstSettings();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Back]))
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    // Initializing all variables for images in the toolbar
    static constexpr size_t size{4}; // number of pictures
    int PngRenderedCnt = 0;
    static bool loaded_png[size]{};
    static int image_height[size];
    static int image_width[size];
    static GLuint image_texture[size];
    static constexpr float iconsSacle{0.5f};

    // The order matters because of the counter for the images !!!
    static const unsigned char *imagesNames[] = {
        PlayButton_png, PauseButton_png, SaveButton_png, ResetButton_png};
    static const unsigned int imagesLen[] = {
        PlayButton_png_len, PauseButton_png_len, SaveButton_png_len,
        ResetButton_png_len};
    // Load the images for the SideBarMenu
    for (int i = 0; i < size; i++)
      if (!loaded_png[i]) {
        if (LoadTextureFromHeader(imagesNames[i], imagesLen[i],
                                  &image_texture[i], &image_width[i],
                                  &image_height[i]))
          loaded_png[i] = true;
        else
          fmt::println("Error Loading Png #{}.", i);
      }

    ImGui::SetCursorPosY(windowSize.y * 0.05f);

    if (flagPaused) {
      // ######################## Buttonstripe
      if (!devices.empty()) {
        if (!sampler.has_value()) {
            ImGui::SetCursorPosY(windowSize.y * 0.05f);
          PngRenderedCnt = 0;
          set_button_style_to(config,
                              "start"); // Start Button
          if (ImGui::ImageButton(
                  appLanguage[Key::Start],
                  (void *)(intptr_t)image_texture[PngRenderedCnt],
                  ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                         image_height[PngRenderedCnt] * iconsSacle))) {
            sampler.emplace(deviceManager, std::move(devices));
            flagPaused = false;
            flagDataNotSaved = true;
          }
          ImGui::PopStyleColor(3);
        }
      }
    } else {
      PngRenderedCnt = 1;
      // ############################ Stop Button
        ImGui::SetCursorPosY(windowSize.y * 0.05f);
      set_button_style_to(config, "stop");
      if (ImGui::ImageButton(appLanguage[Key::Stop],
                             (void *)(intptr_t)image_texture[PngRenderedCnt],
                             ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                                    image_height[PngRenderedCnt] * iconsSacle)))
        flagPaused = true;
      ImGui::PopStyleColor(3);
    }
    if (flagPaused) {
      ImGui::SameLine();

      // Start/reset the measurement when the measurement is paused,
      // followed by a query as to whether the old data should be saved
      if (sampler.has_value()) {
        ImGui::SetCursorPosY(windowSize.y * 0.05f);
        PngRenderedCnt = 0;
        ImGui::SameLine();
        set_button_style_to(config, "start");
        if (ImGui::ImageButton(
                appLanguage[Key::Continue],
                (void *)(intptr_t)image_texture[PngRenderedCnt],
                ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                       image_height[PngRenderedCnt] * iconsSacle))) {
          flagPaused = false;
          flagDataNotSaved = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGui::SetCursorPosY(windowSize.y * 0.05f);
        PngRenderedCnt = 3;
        set_button_style_to(config, "stop");
        if (ImGui::ImageButton(
                appLanguage[Key::Reset],
                (void *)(intptr_t)image_texture[PngRenderedCnt],
                ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                       image_height[PngRenderedCnt] * iconsSacle))) {
          if (flagDataNotSaved) {
            ImGui::OpenPopup(appLanguage[Key::Reset_q]);
          } else {
            rstSettings();
            flagPaused = true;
          }
        }
        ImGui::PopStyleColor(3);
      }
      ImGui::SameLine();

      // gray out "Save" button when pop-up is open
      ImGui::SetCursorPosY(windowSize.y * 0.05f);
      const bool pushStyle = ImGui::IsPopupOpen("Save recorded data");
      if (pushStyle)
        ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
      PngRenderedCnt = 2;
      if (ImGui::ImageButton(
              appLanguage[Key::Save],
              (void *)(intptr_t)image_texture[PngRenderedCnt],
              ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                     image_height[PngRenderedCnt] * iconsSacle))) {
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
    } else {
      ImGui::SetCursorPosY(windowSize.y * 0.05f);
      PngRenderedCnt = 2;
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
      ImGui::ImageButton(appLanguage[Key::Save],
                         (void *)(intptr_t)image_texture[PngRenderedCnt],
                         ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                                image_height[PngRenderedCnt] * iconsSacle));
      ImGui::PopStyleColor();
    }
    ImGui::EndChild(); // end child "Buttonstripe"
    // ############################ Settings Menu
    std::string settingstitle =
        load_json<std::string>(language, "settings", "title");
    if (open_settings) {
      ImGui::OpenPopup(settingstitle.c_str());
      open_settings = false;
    }
    if (ImGui::BeginPopupModal(settingstitle.c_str(), nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      popup_settings(config, language, configpath);
      ImGui::EndPopup();
    }
    // Generate training data popup
    if (open_generate_training_data)
      generateTrainingData(open_generate_training_data, captureData,
                           savedFileNames, config);

    // ############################### VCDS Menu
    if (open_VCDS) {
      ImGui::OpenPopup("Generiere Trainingsdaten");
      open_VCDS = false;
    }
    if (ImGui::BeginPopupModal("Generiere Trainingsdaten", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      popup_create_training_data_select(config, upload_success);
      ImGui::EndPopup();
    }

    // ############################ addPlots("Recording the data", ...)
    ImGui::Dummy({0.f, windowSize.y * .01f});
    SetMainWindowStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, windowSize.x * .011f);
    ImGui::BeginChild("Record Data",
                      {windowSize.x * 0.76f, windowSize.y * 0.65f},
                      ImGuiChildFlags_Border);

    addPlots("Recording the data", flagPaused, [&xmax_paused](double x_max) {
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
    ImGui::EndChild(); // end child Record Data
    ImGui::PopStyleVar();

    SetupImGuiStyle(false, 0.99f, config);
    // ############################ Devicelist
    SetDeviceMenuStyle();

    ImGui::Dummy({0.f, windowSize.y * .01f});
    ImGui::BeginChild("Devicelist");
    ImGui::Dummy({windowSize.x * .36f, 0.f});
    ImGui::SameLine();
    ImGui::Text(appLanguage[Key::Devices_found]);
    devicesList();
    ImGui::EndChild(); // end child "Devicelist"
    ImGui::EndChild(); // end child "Right Side"
    ImGui::End();
  };

  ImGuiInstance window{1280, 760,
                       fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                   CMakeGitVersion::Project::Version)};
  while (window.run(render)) {
  }
  return 0;
}