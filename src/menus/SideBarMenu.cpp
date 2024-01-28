#include "../jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

// Function to set the SideBarMenu in the main.cpp // right now this is only the
// first version

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
    ImGui:: SetCursorPos(ImVec2(0, ImGui::GetIO().DisplaySize.y * 0.06f)); 

    ImGui::BeginChild("SideBarMenu", ImVec2(ImGui::GetIO().DisplaySize.x * 0.18f, ImGui::GetIO().DisplaySize.y * 0.93f), true); 

  if (ImGui::BeginMenu(
          load_json<std::string>(language, "menubar", "menu", "label")
              .c_str())) {
    if (ImGui::BeginMenu(load_json<std::string>(language, "menubar", "menu",
                                                "language_option")
                             .c_str())) {
      for (const auto &lang : availableLanguages) {
        if (ImGui::MenuItem(lang.c_str())) {
          config["language"] = lang;
          write_json_file(configpath, config);
        }
      }

      ImGui::EndMenu();
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
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Diagnostics")) {
    if (ImGui::BeginMenu("Compression")) {
      ImGui::MenuItem("Analyze current waveform");
      if (ImGui::MenuItem("Generate training data"))
        open_generate_training_data = true;

      ImGui::EndMenu();
    }

    // greyed out color style
    ImGui::PushStyleColor(
        ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

    ImGui::MenuItem("Timing-Belt");
    ImGui::MenuItem("Fuel-Delivery-Pump");
    ImGui::MenuItem("Common-Rail-Pressure");
    ImGui::PopStyleColor(2);
    ImGui::PushStyleColor(ImGuiCol_Text,
                          load_json<Color>(config, "text", "color", "normal"));

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu(
          load_json<std::string>(language, "menubar", "help", "label")
              .c_str())) {
    if (ImGui::MenuItem(load_json<std::string>(language, "helplink").c_str())) {
      system(("start " + load_json<std::string>(config, "helplink")).c_str());
    }

    ImGui::EndMenu();
  }

  mainMenuBarSize = ImGui::GetItemRectSize();
  ImGui::EndChild();
}