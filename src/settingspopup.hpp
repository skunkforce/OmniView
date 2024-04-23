#pragma once

#include "jasonhandler.hpp"
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

static void popup_settings(nlohmann::json &config, nlohmann::json &language,
                           std::string const &configpath) {
  static float fontscale;
  static nlohmann::json newconfig = 0;
  if (newconfig == 0) {
    newconfig = config;
  }
  if (fontscale < load_json<float>(newconfig, "text", "minscale")) {
    fontscale = load_json<float>(newconfig, "text", "minscale");
  }
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = fontscale;
  std::string fontscalestring = fmt::format(
      "{} {:.1f}", load_json<std::string>(language, "settings", "fontsize"),
      fontscale);
  ImGui::TextUnformatted(fontscalestring.c_str());
  ImGui::SameLine();
  if (ImGui::Button("+")) {
    fontscale += 0.1f;
    newconfig["text"]["scale"] = fontscale;
  }
  ImGui::SameLine();
  if (ImGui::Button("-")) {
    fontscale -= 0.1f;
    newconfig["text"]["scale"] = fontscale;
  }

  if (ImGui::Button(
          load_json<std::string>(language, "button", "save").c_str())) {
    write_json_file(configpath, newconfig);
    config = newconfig;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(
          load_json<std::string>(language, "button", "cancel").c_str())) {
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(
          load_json<std::string>(language, "button", "restore").c_str())) {
    newconfig = config;
    fontscale = load_json<float>(newconfig, "text", "scale");
  }
}