#include <cmake_git_version/version.hpp>
#include <iostream>
#include <string>
#include <vector>
#define IMGUI_DEFINE_MATH_OPERATORS
#define STB_IMAGE_IMPLEMENTATION
#include "style.hpp"
#include "../stb_image/stb_image.h"
#include "imgui_internal.h"
#include "jasonhandler.hpp"
#include "languages.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"

void set_side_menu(const nlohmann::json &config) {

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

void set_toolbar(const nlohmann::json &config, bool &flagPaused) {

  auto windowSize{ImGui::GetIO().DisplaySize};

  // begin Toolbar ############################################
  ImGui::BeginChild("Buttonstripe", {-1.f, windowSize.y * .1f}, false,
                    ImGuiWindowFlags_NoScrollbar);

  if (flagPaused) {
    // ######################## Buttonstripe
    if (!devices.empty())
      if (!sampler.has_value()) {
        if (ImGui::Button(
                appLanguage[Key::Start])) {
          sampler.emplace(deviceManager, std::move(devices));
          flagPaused = false;
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
        for (auto &device : sampler->sampleDevices) {
          device.first->send(Omniscope::Start{});
        }
      }
      if (ImGui::Button(
              appLanguage[Key::Reset])) {
          flagPaused = true;
        }
      }
    }
  ImGui::EndChild(); // end child "Buttonstripe"
}
