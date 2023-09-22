#include <filesystem>
#include <fmt/format.h>
#include <imgui.h>
#include <vector>

std::string getSubdirectoriesInFolder(std::string const &saves_folder_path) {
  std::vector<std::string> subdirectories;
  std::filesystem::path savesPath(saves_folder_path);

  if (std::filesystem::exists(savesPath) &&
      std::filesystem::is_directory(savesPath)) {
    for (const auto &entry : std::filesystem::directory_iterator(savesPath)) {
      if (std::filesystem::is_directory(entry)) {
        subdirectories.push_back(entry.path().filename().string());
      }
    }
  }

  static int selectedOption = 0;

  if (!subdirectories.empty()) {
    // Erstellen eines Arrays von C-Strings (char*)
    char **vins = new char *[subdirectories.size()];
    for (size_t i = 0; i < subdirectories.size(); ++i) {
      vins[i] = strdup(subdirectories[i].c_str());
    }

    // Verwendung von vins (char* array) mit ImGui
    if (ImGui::Combo("Bekannte Fahrzeuge", &selectedOption, vins,
                     static_cast<int>(subdirectories.size()))) {
      // Hier können Sie die ausgewählte Option verwenden
      const std::string &selectedFolder = subdirectories[selectedOption];
      for (size_t i = 0; i < subdirectories.size(); ++i) {
        free(vins[i]); // Speicher für C-Strings freigeben
      }
      delete[] vins; // Speicher für das Array freigeben
      return selectedFolder;
    }

    // Freigabe des Arrays von C-Strings (char*) nach der Verwendung mit ImGui
    for (size_t i = 0; i < subdirectories.size(); ++i) {
      free(vins[i]);
    }
    delete[] vins;
  }

  return "Neues Fahrzeug";
}
