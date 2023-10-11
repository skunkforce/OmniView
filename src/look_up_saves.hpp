#pragma once

#include "jasonhandler.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <vector>

std::string
getSubdirectoriesInFolder(nlohmann::json language,
                          std::filesystem::path const &saves_folder_path) {
  std::vector<std::string> subdirectories;
  subdirectories.push_back(
      load_json<std::string>(language, "savepopup", "new_car"));
  if (std::filesystem::exists(saves_folder_path) &&
      std::filesystem::is_directory(saves_folder_path)) {

    for (const auto &entry :
         std::filesystem::directory_iterator(saves_folder_path)) {
      if (std::filesystem::is_directory(entry)) {
        subdirectories.push_back(entry.path().filename().string());
      }
    }
  }

  static int selectedOption = 0;
  static std::string selectedFolder =
      load_json<std::string>(language, "savepopup", "new_car");
  if (!subdirectories.empty()) {

    // Erstellen eines Arrays von C-Strings (char*)
    char **vins = new char *[subdirectories.size()];
    for (size_t i = 0; i < subdirectories.size(); ++i) {
      vins[i] = strdup(subdirectories[i].c_str());
    }

    // Verwendung von vins (char* array) mit ImGui
    if (ImGui::Combo(
            load_json<std::string>(language, "savepopup", "known_cars").c_str(),
            &selectedOption, vins, static_cast<int>(subdirectories.size()))) {
      selectedFolder = subdirectories[selectedOption];
    }
    static char inputvin[18];
    const std::string newcar =
        load_json<std::string>(language, "savepopup", "new_car");

    if (selectedOption == 0) {
      ImGui::InputText(
          load_json<std::string>(language, "input", "fin", "label").c_str(),
          inputvin, sizeof(inputvin));
      selectedFolder = inputvin;
    }
  }

  return selectedFolder;
}
std::string select_combo_from_json(nlohmann::json const &language,
                                   std::string const &key,
                                   int &selectedOption) {
  std::string selectedmeasure;
  std::vector<std::string> options;
  for (const auto &item : language[key]) {
    options.push_back(item);
  }

  // Erstellen eines Arrays von C-Strings (char*)
  char **option_c = new char *[options.size()];
  for (size_t i = 0; i < options.size(); ++i) {
    option_c[i] = strdup(options[i].c_str());
  }

  std::string combo_name = "##" + key;
  ImGui::Combo(combo_name.c_str(), &selectedOption, option_c,
               static_cast<int>(options.size()));

  selectedmeasure = options[selectedOption];

  return selectedmeasure;
}
