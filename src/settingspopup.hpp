#ifndef SETTINGS_H
#define SETTINGS_H

#include "jasonhandler.hpp"
#include "languages.hpp"
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

static void popup_settings(nlohmann::json &config, nlohmann::json &language,
                           std::string const &configpath, int &title) {

  static float tempfontscale = config["text"]["scale"];
  static std::map<Key, const char *> tempLan = appLanguage;
  static nlohmann::json newconfig = config;

  ImGuiIO &io = ImGui::GetIO();

  ImGui::Text(appLanguage[Key::SettingsText]);
  ImGui::Text("                            ");

  // Create and initialize customer_id variable
  static char customer_id[20] = "";

  static bool is_initialized =
      false; // Track if customer_id has been initialized

  static bool isempty = false;

  // Initialize customer_id only once
  if (!is_initialized) {
    std::string temp = config["Costumer_id"].get<std::string>();
    std::copy(temp.begin(), temp.end(), customer_id);
    if (temp.empty()) {
      isempty = true;
    }
    customer_id[temp.size()] = '\0'; // Ensure null termination
    is_initialized = true;
  }

  if (isempty) {
    // If customer_id is an empty string, allow user to enter a new ID
    ImGui::InputText(fmt::format("{}", appLanguage[Key::Costumer_id]).c_str(),
                     customer_id, sizeof(customer_id));
  } else {
    // If customer_id is not empty, display it as a non-editable field
    ImGui::Text(
        fmt::format("{}: {}", appLanguage[Key::Costumer_id], customer_id)
            .c_str());
  }

  // Save the potentially updated customer_id back to newconfig
  newconfig["Costumer_id"] = std::string(customer_id);
  // for comparison:     //load_json<std::string>(language, "settings",
  // "buttonexplain").c_str()
  // new idea: load id from config file
  // std::string KnownCostumer_id = load_json<std::string>(config,
  // "Costumer_id"); ImGui::InputText(fmt::format("{}",
  // appLanguage[Key::Costumer_id]).c_str(), &KnownCostumer_id);
  // newconfig["Costumer_id"]= KnownCostumer_id;
  // error: key not found initially in config, does not save when closed

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
    newconfig["Costumer_id"] = std::string(customer_id);
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