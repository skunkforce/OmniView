#pragma once

#include "jasonhandler.hpp"
#include "languages.hpp"
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
  std::string fontscalestring =
      fmt::format("{} {:.1f}", appLanguage[Key::FontSize], fontscale);

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

  // ####################################################################################
  //                           Button Size
  // ####################################################################################
  static float ButtonSizeX = load_json<float>(config, "button", "sizex");
  static float ButtonSizeY = load_json<float>(config, "button", "sizey");
  ImGui::TextUnformatted(appLanguage[Key::ButtonExplain]);

  ImGui::TextUnformatted(appLanguage[Key::WindowWidth]);
  ImGui::SameLine();

  float oldButtonSizeX = ButtonSizeX;
  float oldButtonSizeY = ButtonSizeY;
  ImGui::InputFloat("X##ButtonSizeX", &ButtonSizeX);

  ImGui::SameLine();
  if (ImGui::Button("X+")) {
    ButtonSizeX += 10.0f;
  }

  ImGui::SameLine();
  if (ImGui::Button("X-")) {
    ButtonSizeX -= 10.0f;
  }
  ImGui::TextUnformatted(appLanguage[Key::WindowHeight]);
  ImGui::SameLine();

  ImGui::InputFloat("Y##ButtonSizeY",
                    &ButtonSizeY); // Eingabe-Float für Y-Größe
  ImGui::SameLine();
  if (ImGui::Button("Y+")) {
    ButtonSizeY += 10.0f;
  }

  ImGui::SameLine();
  if (ImGui::Button("Y-")) {
    ButtonSizeY -= 10.0f;
  }

  if (ImGui::Button(appLanguage[Key::Save], ImVec2(ButtonSizeX, ButtonSizeY))) {

    write_json_file(configpath, newconfig);
    config = newconfig;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Cancel],
                    ImVec2(ButtonSizeX, ButtonSizeY))) {
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Restore],
                    ImVec2(ButtonSizeX, ButtonSizeY))) {
    newconfig = config;
    fontscale = load_json<float>(newconfig, "text", "scale");
    ButtonSizeX = load_json<float>(config, "button", "sizex");
    ButtonSizeY = load_json<float>(config, "button", "sizey");
  }
  if (oldButtonSizeX != ButtonSizeX || oldButtonSizeY != ButtonSizeY) {
    oldButtonSizeX = ButtonSizeX;
    oldButtonSizeY = ButtonSizeY;
    newconfig["button"]["sizey"] = ButtonSizeY;
    newconfig["button"]["sizex"] = ButtonSizeX;
  }
}
