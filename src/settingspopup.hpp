#pragma once

#include "jasonhandler.hpp"
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

static void popup_settings(nlohmann::json &config, nlohmann::json &language,
                           std::string const &configpath) {

  static float tempfontscale;

  ImGuiIO &io = ImGui::GetIO();

   if (tempfontscale < load_json<float>(config, "text", "minscale")) {
    tempfontscale = load_json<float>(config, "text", "minscale");
  }

  std::string fontscalestring = fmt::format(
      "{} {:.1f}", load_json<std::string>(language, "settings", "fontsize"),
      tempfontscale);
  ImGui::TextUnformatted(fontscalestring.c_str());
  ImGui::SameLine();


  if (ImGui::Button("+")) {
    tempfontscale += 0.1f;
    io.FontGlobalScale = tempfontscale;
  }
  ImGui::SameLine();
  if (ImGui::Button("-") && (tempfontscale > 1.0f)) {
    tempfontscale -= 0.1f;
    io.FontGlobalScale = tempfontscale;
  }

  if (ImGui::Button(
          load_json<std::string>(language, "button", "save").c_str())) {
    config["text"]["scale"] = tempfontscale; 
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(
          load_json<std::string>(language, "button", "cancel").c_str())) {
    io.FontGlobalScale = config["text"]["scale"]; 
    tempfontscale = config["text"]["scale"]; 
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(
          load_json<std::string>(language, "button", "restore").c_str())) {
    io.FontGlobalScale = config["text"]["minscale"];
    tempfontscale = config["text"]["minscale"]; 
  }
}