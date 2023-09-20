#pragma once

// #include "jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>

void open_help_popup(nlohmann::json const &config,
                     nlohmann::json const &language) {

  ImGui::SetItemDefaultFocus();
  ImGui::Text(load_json<std::string>(language, "helplink").c_str());

  if (ImGui::Button(load_json<std::string>(config, "helplink").c_str())) {

    system(("start " + load_json<std::string>(config, "helplink")).c_str());

    ImGui::CloseCurrentPopup();
  }
  if (ImGui::Button(
          load_json<std::string>(language, "button", "back").c_str())) {

    ImGui::CloseCurrentPopup();
  }
}
