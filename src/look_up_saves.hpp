#pragma once

#include "jasonhandler.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <vector>

namespace fs = std::filesystem;
inline std::string getSubdirectoriesInFolder(nlohmann::json language,
                                      fs::path const &saves_folder_path,
                                      char scantype[255] = 0,
                                      char inputvin[18] = 0,
                                      char mileage[10] = 0) {
  bool isTxtInptFieldsEmpty = false;
  if (scantype != 0 && inputvin != 0 && mileage != 0)
    isTxtInptFieldsEmpty = true;

  std::vector<std::string> subdirectories;
  subdirectories.push_back("New Car");

  if (fs::exists(saves_folder_path) && fs::is_directory(saves_folder_path)) {
    for (const auto &entry : fs::directory_iterator(saves_folder_path))
      if (fs::is_directory(entry))
        subdirectories.push_back(entry.path().filename().string());
  }

  static int selectedOption = 0;
  static std::string selectedFolder = "New Car";
  if (!subdirectories.empty()) {
    // Creating an array of C strings (char*)
    char **vins = new char *[subdirectories.size()];
    for (size_t i = 0; i < subdirectories.size(); ++i)
      vins[i] = strdup(subdirectories[i].c_str());

    if (isTxtInptFieldsEmpty) {
      const std::string newcar = "New Car";
      ImGui::Separator();
      ImGui::NewLine();
      ImGui::Text("Additional Information");
      ImGui::NewLine();

      ImGui::Combo("Known Cars", &selectedOption, vins,
                   static_cast<int>(subdirectories.size()));

      ImGui::InputText("Measurement", scantype, IM_ARRAYSIZE(scantype));

      if (selectedOption == 0) {
        ImGui::InputText("Fin/Vin", inputvin, IM_ARRAYSIZE(inputvin),
                         ImGuiInputTextFlags_CharsUppercase |
                             ImGuiInputTextFlags_CharsNoBlank |
                             ImGuiInputTextFlags_None);
        selectedFolder = inputvin;
      }

      ImGui::InputText("Mileage", mileage, IM_ARRAYSIZE(mileage));
    } else {
      // Using vins (char* array) with ImGui
      ImGui::Combo("Known Cars", &selectedOption, vins,
                   static_cast<int>(subdirectories.size()));

      if (selectedOption == 0) {
        ImGui::InputText("Fin/Vin", inputvin, IM_ARRAYSIZE(inputvin));
        selectedFolder = inputvin;
      }

      static char VIN[18];
      const std::string newcar = "New Car";

      ImGui::Separator();
      ImGui::Text("Additional Information");
      ImGui::NewLine();
      ImGui::InputText("Measurement", scantype, IM_ARRAYSIZE(scantype));
      ImGui::InputText("Mileage", mileage, IM_ARRAYSIZE(mileage));
    }
  }

  return selectedFolder;
}

inline std::string select_combo_from_json(nlohmann::json const &language,
                                   std::string const &key,
                                   int &selectedOption) {
  std::string selectedmeasure;
  std::vector<std::string> options;
  for (const auto &item : language[key]) {
    options.push_back(item);
  }

  // Creating an array of C strings (char*)
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
