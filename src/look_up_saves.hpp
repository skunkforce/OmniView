#pragma once
#include "jasonhandler.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include "languages.hpp"
#include <vector>

inline std::string getSubdirectoriesInFolder(nlohmann::json language,
                                      fs::path const &saves_folder_path,
                                      char scantype[255] = 0,
                                      char inputvin[19] = 0,
                                      char mileage[10] = 0) {
  bool isTxtInptFieldsEmpty = false;
  if (scantype != 0 && inputvin != 0 && mileage != 0)
    isTxtInptFieldsEmpty = true;

  std::vector<std::string> subdirectories;
  subdirectories.push_back(appLanguage[Key::New_Car]);

  if (fs::exists(saves_folder_path) && fs::is_directory(saves_folder_path)) {
    for (const auto &entry : fs::directory_iterator(saves_folder_path))
      if (fs::is_directory(entry))
        subdirectories.push_back(entry.path().filename().string());
  }

  static int selectedOption = 0;
  static std::string selectedFolder = appLanguage[Key::New_Car];
  if (!subdirectories.empty()) {
    // Creating an array of C strings (char*)
    char **vins = new char *[subdirectories.size()];
    for (size_t i = 0; i < subdirectories.size(); ++i)
      vins[i] = strdup(subdirectories[i].c_str());

    if (isTxtInptFieldsEmpty) {
      const std::string newcar = appLanguage[Key::New_Car];
      ImGui::Separator();
      ImGui::NewLine();
      ImGui::Text(appLanguage[Key::Additional_Information]);
      ImGui::NewLine();

      ImGui::Combo(appLanguage[Key::Known_Car], &selectedOption, vins,
                   static_cast<int>(subdirectories.size()));

      ImGui::InputText(appLanguage[Key::Measurement], scantype, 255);
      if (selectedOption == 0) {
        ImGui::InputText("Fin/Vin", inputvin, 19,
                         ImGuiInputTextFlags_CharsUppercase |
                             ImGuiInputTextFlags_CharsNoBlank |
                             ImGuiInputTextFlags_None);
        selectedFolder = inputvin;
      }

      ImGui::InputText(appLanguage[Key::Mileage], mileage, 10);

      // Using vins (char* array) with ImGui
      }else {
      ImGui::Combo(appLanguage[Key::Known_Car], &selectedOption, vins,
                   static_cast<int>(subdirectories.size()));

      if (selectedOption == 0) {
        ImGui::InputText("Fin/Vin", inputvin, 19);
        selectedFolder = inputvin;
      }
      static char VIN[18];
      const std::string newcar = appLanguage[Key::New_Car];

      ImGui::Separator();
      ImGui::Text(appLanguage[Key::Additional_Information]);
      ImGui::NewLine();
      ImGui::InputText(appLanguage[Key::Measurement], scantype, 255);
      ImGui::InputText(appLanguage[Key::Mileage], mileage, 10);
    }
  }

  return selectedFolder;
}
inline std::string select_combo_from_json(nlohmann::json const &language,
                                   std::string const &key,
                                   int &selectedOption) {
  std::vector<std::string> options;
  for (const auto &item : language[key]) 
    options.push_back(item);

  // Creating an array of C strings (char*)
  char **option_c = new char *[options.size()];
  for (size_t i = 0; i < options.size(); ++i) 
    option_c[i] = strdup(options[i].c_str());

  std::string combo_name = "##" + key;
  ImGui::Combo(combo_name.c_str(), &selectedOption, option_c,
               static_cast<int>(options.size()));

  return options[selectedOption];
}
