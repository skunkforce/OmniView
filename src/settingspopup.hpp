#ifndef SETTINGS_H
#define SETTINGS_H

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <fmt/format.h>
#include "jasonhandler.hpp"
#include "languages.hpp"

static void popup_settings(nlohmann::json &config,
                           std::string const &configpath, int &title) {

  static float tempfontscale = config["text"]["scale"];
  static std::map<Key, const char *> tempLan = appLanguage;
  static nlohmann::json newconfig = config;

  ImGuiIO &io = ImGui::GetIO();

  ImGui::Text(appLanguage[Key::Set_id_in_setting]);
  ImGui::Text("                            ");

  if (tempfontscale < load_json<float>(config, "text", "minscale")) {
    tempfontscale = load_json<float>(config, "text", "minscale");
  }

  std::string fontscalestring =
      fmt::format("{} {:.1f}", appLanguage[Key::FontSize], tempfontscale);

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

  static bool treeopen = true;

  if (!treeopen) {
    ImGui::SetNextItemOpen(false);
    treeopen = true;
  }
  if (ImGui::TreeNode(appLanguage[Key::LanOption]) && treeopen) {
    if (ImGui::Button(appLanguage[Key::English]) && englishLan != appLanguage) {
      appLanguage = englishLan;
      title = 0; // English index
      treeopen = false;
    }
    if (ImGui::Button(appLanguage[Key::German]) && germanLan != appLanguage) {
      appLanguage = germanLan;
      title = 1; // German index
      treeopen = false;
    }
    ImGui::TreePop();
  }

  if (ImGui::Button(appLanguage[Key::Save])) {
    newconfig["text"]["scale"] = tempfontscale;
    newconfig["text"]["active_language"] =
        appLanguage == germanLan ? "German" : "English";
    config = newconfig;
    write_json_file(configpath, config);
    tempLan = appLanguage;
    treeopen = false;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Back])) {
    io.FontGlobalScale = config["text"]["scale"];
    tempfontscale = config["text"]["scale"];
    appLanguage = tempLan;
    treeopen = false;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button(appLanguage[Key::Reset])) {
    io.FontGlobalScale = config["text"]["minscale"];
    tempfontscale = config["text"]["minscale"];
    appLanguage = germanLan;
    tempLan = germanLan;
    treeopen = false;
  }
}

#endif // SETTING_H