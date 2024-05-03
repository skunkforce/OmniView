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

  static float tempfontscale;

  ImGuiIO &io = ImGui::GetIO();

  ImGui::Text(appLanguage[Key::SettingsText]);
  ImGui::Text("                            ");

   if (tempfontscale < load_json<float>(config, "text", "minscale")) {
    tempfontscale = load_json<float>(config, "text", "minscale");
  }

  std::string fontscalestring = fmt::format(
      "{} {:.1f}", appLanguage[Key::FontSize],
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

  if (ImGui::TreeNode(appLanguage[Key::LanOption])) {
    if (ImGui::Button(appLanguage[Key::English])) {
      appLanguage = englishLan;
    }
    if (ImGui::Button(appLanguage[Key::German])) {
      appLanguage = germanLan;
    }
    ImGui::TreePop();
  }

  if (ImGui::Button(
          appLanguage[Key::Save])) {
    config["text"]["scale"] = tempfontscale; 
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(
          appLanguage[Key::Back])) {
    io.FontGlobalScale = config["text"]["scale"]; 
    tempfontscale = config["text"]["scale"]; 
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Reset])) {
    io.FontGlobalScale = config["text"]["minscale"];
    tempfontscale = config["text"]["minscale"]; 
    appLanguage = germanLan;
  }
}

#endif // SETTING_H