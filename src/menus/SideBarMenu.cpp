#include <ImGuiInstance/ImGuiInstance.hpp>

// Function to set the SideBarMenu in the main.cpp // right now this is only the first version 


void SetSideBarMenu(nlohmann::json &language, std::vector<std::string> &availableLanguages, nlohmann::json &config, const std::string &configpath, bool &open_settings, std::optional<OmniscopeSampler> &sampler, 
std::vector<std::shared_ptr<OmniscopeDevice>> &devices, OmniscopeDeviceManager &deviceManager, std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData, bool &flagPaused){

    ImGui::BeginMainMenuBar();


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
}