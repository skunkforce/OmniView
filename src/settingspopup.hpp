#ifndef SETTINGS_H
#define SETTINGS_H

#include "jasonhandler.hpp"
#include "languages.hpp"
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

static void popup_settings(nlohmann::json &config, nlohmann::json &language,
                           std::string const &configpath) {
  static nlohmann::json newconfig = config;
  static float fontscale;

  ImGui::Text(appLanguage[Key::SettingsText]);
  ImGui::Text("                            ");

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

  if (ImGui::TreeNode(appLanguage[Key::LanOption])) {
    if (ImGui::Button(appLanguage[Key::English])) {
      appLanguage = englishLan;
    }
    if (ImGui::Button(appLanguage[Key::German])) {
      appLanguage = germanLan;
    }
    ImGui::TreePop();
  }

  if (ImGui::Button(appLanguage[Key::Save])) {
    write_json_file(configpath, newconfig);
    config = newconfig;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Back])) {
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Reset])) {
    newconfig = config;
    fontscale = load_json<float>(newconfig, "text", "scale");
    appLanguage = germanLan;
  }
}

#endif // SETTING_H