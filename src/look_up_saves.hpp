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
      // Hier können Sie die ausgewählte Option verwenden
      selectedFolder = subdirectories[selectedOption];
      // Speicher für das Array freigeben
    }

    /*// Freigabe des Arrays von C-Strings (char*) nach der Verwendung mit ImGui
    for (size_t i = 0; i < subdirectories.size(); ++i) {
      free(vins[i]);
    }
    delete[] vins;*/
  }

  return selectedFolder;
}
