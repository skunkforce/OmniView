#include "../jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

// Function to set the SideBarMenu in the main.cpp // right now this is only the
// first version

void ShowSearchDevices() { // Search Devices Menu
  ImGui::Text("Bereich für die Suche nach Geräten");
}

void SetSideBarMenu(
    nlohmann::json &language, std::vector<std::string> &availableLanguages,
    nlohmann::json &config, const std::string &configpath, bool &open_settings,
    std::optional<OmniscopeSampler> &sampler,
    std::vector<std::shared_ptr<OmniscopeDevice>> &devices,
    OmniscopeDeviceManager &deviceManager,
    std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData,
    bool &flagPaused, bool &open_generate_training_data,
    ImVec2 &mainMenuBarSize) {

  // set the menu to the left side of the window ; important size in percentage!
  ImGui::SetCursorPos(ImVec2(0, ImGui::GetIO().DisplaySize.y * 0.06f));

  ImGui::BeginChild("SideBarMenu",
                    ImVec2(ImGui::GetIO().DisplaySize.x * 0.18f,
                           ImGui::GetIO().DisplaySize.y * 0.93f),
                    true);

  if (ImGui::Button("Suche Geräte")) {
    ShowSearchDevices();
  }

  // Changing the Menustructure to a TreeNode Structure

  static bool showSubmenu1 = false;
  // First Menupoint shown as a button
  if (ImGui::Button(load_json<std::string>(language, "menubar", "menu", "label")
                        .c_str())) {
    // Aktion bei Klick auf Menüpunkt 1
    showSubmenu1 = !showSubmenu1;
  }

  // if button has been clicked to that :

  if (showSubmenu1) {
    if (ImGui::TreeNode(load_json<std::string>(language, "menubar", "menu",
                                               "language_option")
                            .c_str())) {
      for (const auto &lang : availableLanguages) {
        if (ImGui::Button(lang.c_str())) {
          config["language"] = lang;
          write_json_file(configpath, config);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Layout")) {
        ImGui::TreePop(); // Closes the TreeNode
      }
    }
    if (ImGui::MenuItem(
            load_json<std::string>(language, "menubar", "menu", "settings")
                .c_str())) {
      open_settings = true;
    }

    if (ImGui::MenuItem(
            load_json<std::string>(language, "menubar", "menu", "reset")
                .c_str())) {
      sampler.reset();
      devices.clear();
      deviceManager.clearDevices();
      captureData.clear();
      flagPaused = true;
    }

    if (ImGui::MenuItem(
            fmt::format("Version: {}", CMakeGitVersion::VersionWithGit)
                .c_str())) {
    }
  }

  static bool showSubmenu2 = false;
  // First Menupoint shown as a button
  if (ImGui::Button("Diagnostics")) {
    // Aktion bei Klick auf Menüpunkt 1
    showSubmenu2 = !showSubmenu2;
  }

  if (showSubmenu2) {
    if (ImGui::TreeNode("Compression")) {
      ImGui::MenuItem("Analyze current waveform");
      if (ImGui::MenuItem("Generate training data"))
        open_generate_training_data = true;

      ImGui::TreePop();
    }

    // greyed out color style
    ImGui::PushStyleColor(
        ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

    if (ImGui::TreeNode("Timing-Belt")) {
      ImGui::MenuItem("Analyze current waveform");
      if (ImGui::MenuItem("Generate training data"))
        open_generate_training_data = true;
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Fuel-Delivery-Pump")) {
      ImGui::MenuItem("Analyze current waveform");
      if (ImGui::MenuItem("Generate training data"))
        open_generate_training_data = true;
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Common-Rail-Pressure")) {
      ImGui::MenuItem("Analyze current waveform");
      if (ImGui::MenuItem("Generate training data"))
        open_generate_training_data = true;
      ImGui::TreePop();
    }

    ImGui::PopStyleColor(2);
    ImGui::PushStyleColor(ImGuiCol_Text,
                          load_json<Color>(config, "text", "color", "normal"));
  }

  static bool showSubmenu3 = false;
  // First Menupoint shown as a button
  if (ImGui::Button(load_json<std::string>(language, "menubar", "help", "label")
                        .c_str())) {
    // Aktion bei Klick auf Menüpunkt 1
    showSubmenu3 = !showSubmenu3;
  }

  if (showSubmenu3) {
    if (ImGui::MenuItem(load_json<std::string>(language, "helplink").c_str())) {
      system(("start " + load_json<std::string>(config, "helplink")).c_str());
    }
  }

  mainMenuBarSize = ImGui::GetItemRectSize();
  ImGui::EndChild();
}