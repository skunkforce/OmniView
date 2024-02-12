
#include "../jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../images/AutoInternLogo_data.h"
#include "../images/DiagnosticsWhite_data.h"
#include "../images/HelpWhite_data.h"
#include "../images/SearchDevicesWhite_data.h"
#include "../images/SettingsWhite_data.h"

// Function to set the SideBarMenu in the main.cpp // first version

void SetSideBarMenu(
    nlohmann::json &language, std::vector<std::string> &availableLanguages,
    nlohmann::json &config, const std::string &configpath, bool &open_settings,
    std::optional<OmniscopeSampler> &sampler,
    std::vector<std::shared_ptr<OmniscopeDevice>> &devices,
    OmniscopeDeviceManager &deviceManager,
    std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData,
    bool &flagPaused, bool &open_generate_training_data,
    ImVec2 &mainMenuBarSize,
    std::map<Omniscope::Id, std::array<float, 3>> &colorMap) {

  // Initializing all variables for images
  static bool loaded_png{false};
  static bool loaded_png1{false};
  static bool loaded_png2{false};
  static bool loaded_png3{false};
  static bool loaded_png4{false};
  static int my_image_width, my_image_width1, my_image_width2, my_image_width3,
      my_image_width4, my_image_width5;
  static int my_image_height, my_image_height1, my_image_height2,
      my_image_height3, my_image_height4, my_image_height5;
  static GLuint my_image_texture, my_image_texture1, my_image_texture2,
      my_image_texture3, my_image_texture4;
  bool ret, ret1, ret2, ret3, ret4;

  // Load the images for the SideBarMenu

  // Load the AIGroup Logo
  if (!loaded_png) {
    ret = LoadTextureFromHeader(AutoInternLogo_png, AutoInternLogo_png_len,
                                &my_image_texture, &my_image_width,
                                &my_image_height);
    if (ret == NULL) {
      fmt::print("Error Loading Png\n");
      loaded_png = false;
    } else {
      loaded_png = true;
    }
  }
  // SearchforDevices
  if (!loaded_png1) {
    ret1 = LoadTextureFromHeader(SearchDevicesWhite_png,
                                 SearchDevicesWhite_png_len, &my_image_texture1,
                                 &my_image_width1, &my_image_height1);
    if (ret1 == NULL) {
      fmt::print("Error Loading Png\n");
      loaded_png1 = false;
    } else {
      loaded_png1 = true;
    }
  }
  // Diagnostics
  if (!loaded_png2) {
    ret2 = LoadTextureFromHeader(DiagnosticsWhite_png, DiagnosticsWhite_png_len,
                                 &my_image_texture2, &my_image_width2,
                                 &my_image_height2);
    if (ret2 == NULL) {
      fmt::print("Error Loading Png\n");
      loaded_png2 = false;
    } else {
      loaded_png2 = true;
    }
  }
  // Settings
  if (!loaded_png3) {
    ret3 = LoadTextureFromHeader(SettingsWhite_png, SettingsWhite_png_len,
                                 &my_image_texture3, &my_image_width3,
                                 &my_image_height3);
    if (ret3 == NULL) {
      fmt::print("Error Loading Png\n");
      loaded_png3 = false;
    } else {
      loaded_png3 = true;
    }
  }
  // Help
  if (!loaded_png4) {
    ret4 = LoadTextureFromHeader(HelpWhite_png, HelpWhite_png_len,
                                 &my_image_texture4, &my_image_width4,
                                 &my_image_height4);
    if (ret4 == NULL) {
      fmt::print("Error Loading Png\n");
      loaded_png4 = false;
    } else {
      loaded_png4 = true;
    }
  }

  // Set the SideBarMenu Colors
  ImGuiStyle &style = ImGui::GetStyle();

  style.Colors[ImGuiCol_Border] =
      ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_BorderShadow] =
      ImVec4(37 / 255.0f, 255 / 255.0f, 43 / 255.0f, 100 / 100.0f);

  // InitDevices after searching for devices
  static constexpr int VID = 0x2e8au;
  static constexpr int PID = 0x000au;

  auto initDevices = [&]() {
    devices = deviceManager.getDevices(VID, PID);
    for (auto &device : devices) {
      auto id = device->getId().value();
      if (!colorMap.contains(id)) {
        ImPlot::PushColormap(ImPlotColormap_Dark);
        auto c = ImPlot::GetColormapColor((colorMap.size() % 7) + 1);
        colorMap[id] = std::array<float, 3>{c.x, c.y, c.z};
        ImPlot::PopColormap();
      }
      auto &color = colorMap[id];
      device->send(
          Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                            static_cast<std::uint8_t>(color[1] * 255),
                            static_cast<std::uint8_t>(color[2] * 255)});
    }
  };

  // set the menu to the left side of the window ; important size in percentage!
  ImGui::SetCursorPos(ImVec2(0, 0));

  ////////////////////////////////////
  ////// Begin the SideBarMenu ///////
  ////////////////////////////////////

  ImGui::BeginChild("SideBarMenu",
                    ImVec2(ImGui::GetIO().DisplaySize.x * 0.18f,
                           ImGui::GetIO().DisplaySize.y),
                    true);

  if (loaded_png) {
    // render the AIGroupLogo
    ImGui::Image((void *)(intptr_t)my_image_texture,
                 ImVec2(my_image_width * 0.5, my_image_height * 0.5));
    ImGui::Text("              ");
  }

  if (loaded_png1) { // search for Devices
    if (!sampler.has_value()) {
      if (ImGui::ImageButton(
              "Search Devices", (void *)(intptr_t)my_image_texture1,
              ImVec2(my_image_width1 * 0.6, my_image_height1 * 0.6))) {
        devices.clear();
        deviceManager.clearDevices();
        initDevices();
      }
    }
  }

  // Changing the Menustructure to a TreeNode Structure
  if (loaded_png2) { // Diagnostics

    static bool showSubmenu2 = false;

    if (ImGui::ImageButton(
            "Diagnostics", (void *)(intptr_t)my_image_texture2,
            ImVec2(my_image_width2 * 0.6, my_image_height2 * 0.6))) {
      // Aktion bei Klick auf Menüpunkt 1
      showSubmenu2 = !showSubmenu2;
    }

    if (showSubmenu2) {

      if (ImGui::TreeNode("Batteriemessung")) {
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            ImVec4(100 / 255.0f, 100 / 255.0f, 100 / 255.0f, 100 / 100.0f));

        ImGui::MenuItem("Analysiere Daten");

        ImGui::PopStyleColor();

        if (ImGui::MenuItem("Generiere Trainingsdaten"))
          open_generate_training_data = true;

        ImGui::TreePop();
      }
    }
  }

  if (loaded_png3) { // Settings

    static bool showSubmenu1 = false;

    if (ImGui::ImageButton(
            "Settings", (void *)(intptr_t)my_image_texture3,
            ImVec2(my_image_width3 * 0.6, my_image_height3 * 0.6))) {
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
        }
        ImGui::TreePop();
      }
      if (ImGui::MenuItem("   Layout")) {
        open_settings = true;
      }
    }
  }
  if (loaded_png4) { // Help

    static bool showSubmenu3 = false;
    // First Menupoint shown as a button

    ret4 = LoadTextureFromFile("../images/HelpWhite.png", &my_image_texture4,
                               &my_image_width4, &my_image_height4);

    if (ImGui::ImageButton(
            "Help", (void *)(intptr_t)my_image_texture4,
            ImVec2(my_image_width4 * 0.6, my_image_height4 * 0.6))) {
      // Aktion bei Klick auf Menüpunkt 1
      showSubmenu3 = !showSubmenu3;
    }

    if (showSubmenu3) {
      if (ImGui::MenuItem(
              load_json<std::string>(language, "helplink").c_str())) {
        system(("start " + load_json<std::string>(config, "helplink")).c_str());
      }
    }
  }

  ImGui::SetCursorPosY(ImGui::GetIO().DisplaySize.y * 0.95f);
  ImGui::Text(
      fmt::format("   Version: {}", CMakeGitVersion::VersionWithGit).c_str());

  mainMenuBarSize = ImGui::GetItemRectSize();
  ImGui::EndChild();
}

// EndofFunction

// Analyses that will be added in a later process :
/*
   if (ImGui::TreeNode("Compression")) {
     ImGui::MenuItem("Analyze current waveform");
     if (ImGui::MenuItem("Generate training data"))
       open_generate_training_data = true;

     ImGui::TreePop();
   }

   // greyed out color style
   // ImGui::PushStyleColor(
   //  ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

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

   // ImGui::PopStyleColor(2);
   // ImGui::PushStyleColor(ImGuiCol_Text,
   // load_json<Color>(config, "text", "color", "normal"));
 } */