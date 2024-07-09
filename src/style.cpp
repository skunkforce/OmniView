#include <cmake_git_version/version.hpp>
#include <iostream>
#include <string>
#include <vector>
#define IMGUI_DEFINE_MATH_OPERATORS
#define STB_IMAGE_IMPLEMENTATION
#include "style.hpp"
#include "../stb_image/stb_image.h"
#include "imagesHeader.hpp"
#include "imgui_internal.h"
#include "jasonhandler.hpp"
#include "languages.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"
#include "popups.hpp"

void set_side_menu(const nlohmann::json &config, bool &open_settings,
                   bool &open_generate_training_data,
                   decltype(captureData) &loadedFiles,
                   std::map<Omniscope::Id, std::string> &loadedFilenames) {

  auto windowSize{ImGui::GetIO().DisplaySize};

  // Start only if devices are available, otherwise search for devices
  if ( // render search for Devices
      !sampler.has_value() &&
      ImGui::Button(
          "Search Device")) {
    devices.clear();
    deviceManager.clearDevices();
    initDevices();
  }
}

void set_toolbar(const nlohmann::json &config, const nlohmann::json &language,
                 bool &flagPaused, const decltype(captureData) &loadedFiles) {

  // variable declaration
  static auto now = std::chrono::system_clock::now();
  static std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  static std::tm now_tm = *std::gmtime(&now_time_t);
  auto windowSize{ImGui::GetIO().DisplaySize};
  static bool flagDataNotSaved = true;
  static decltype(captureData) liveDvcs;

  // begin Toolbar ############################################
  ImGui::BeginChild("Buttonstripe", {-1.f, windowSize.y * .1f}, false,
                    ImGuiWindowFlags_NoScrollbar);
 
  // ############################ Popup Reset
  if (ImGui::BeginPopupModal(appLanguage[Key::Reset_q], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    ImGui::Text(appLanguage[Key::Measure_not_saved]);
    if (ImGui::Button(appLanguage[Key::Continue_del])) {
      rstSettings(loadedFiles);
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
        if (ImGui::Button(
                appLanguage[Key::Start])) {
          sampler.emplace(deviceManager, std::move(devices));
          flagPaused = false;
          flagDataNotSaved = true;
        }
      }
  } else {
    // ############################ Stop Button
    if (ImGui::Button(appLanguage[Key::Stop])) {
      flagPaused = true;
      for (auto &device : sampler->sampleDevices) {
        device.first->send(Omniscope::Stop{});
      }
    }
    ImGui::PopStyleColor(3);
  }
  if (flagPaused) {
    // Start/reset the measurement when the measurement is paused,
    // followed by a query as to whether the old data should be saved
    if (sampler.has_value()) {
    
      if (ImGui::Button(
              appLanguage[Key::Continue])) {
        flagPaused = false;
        flagDataNotSaved = true;
        for (auto &device : sampler->sampleDevices) {
          device.first->send(Omniscope::Start{});
        }
      }
      if (ImGui::Button(
              appLanguage[Key::Reset])) {
        if (flagDataNotSaved) {
          ImGui::OpenPopup(appLanguage[Key::Reset_q]);
        } else {
          rstSettings(loadedFiles);
          flagPaused = true;
        }
      }
    }
  }
  ImGui::EndChild(); // end child "Buttonstripe"
}