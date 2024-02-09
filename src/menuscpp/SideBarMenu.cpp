
/*#include "../jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>*/

// Function to set the SideBarMenu in the main.cpp // right now this is only the
// first



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
    ImVec2 &mainMenuBarSize,
    std::map<Omniscope::Id, std::array<float, 3>> &colorMap) {

  // Colors
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

  ImGui::BeginChild("SideBarMenu",
                    ImVec2(ImGui::GetIO().DisplaySize.x * 0.18f,
                           ImGui::GetIO().DisplaySize.y),
                    true);

  // Load the AIGroup Logo
  int my_image_width = 0;
  int my_image_height = 0;
  GLuint my_image_texture = 0;
  bool ret =
      LoadTextureFromFile("../images/AutoInternLogo.png", &my_image_texture,
                          &my_image_width, &my_image_height);
  IM_ASSERT(ret);

  // render the image
  ImGui::Image((void *)(intptr_t)my_image_texture,
               ImVec2(my_image_width * 0.5, my_image_height * 0.5));

  ImGui::Text("              ");

   my_image_width = 0;
   my_image_height = 0;
   my_image_texture = 0;

  ret = LoadTextureFromFile("../images/SearchDevicesWhite.png", &my_image_texture, &my_image_width, &my_image_height);
  IM_ASSERT(ret);


  if (!sampler.has_value()) {
    if (ImGui::ImageButton("Search Devices", (void *)(intptr_t)my_image_texture,
               ImVec2(my_image_width *0.75 , my_image_height *0.75  ))) {
      devices.clear();
      deviceManager.clearDevices();
      initDevices();
    }
  }

  // Changing the Menustructure to a TreeNode Structure

  static bool showSubmenu2 = false;
  // First Menupoint shown as a 
  my_image_width = 0;
   my_image_height = 0;
   my_image_texture = 0;

  ret = LoadTextureFromFile("../images/DiagnosticsWhite.png", &my_image_texture, &my_image_width, &my_image_height);

  if (ImGui::ImageButton("Diagnostics", (void *)(intptr_t)my_image_texture,
               ImVec2(my_image_width *0.75  , my_image_height *0.75  ))) {
    // Aktion bei Klick auf Menüpunkt 1
    showSubmenu2 = !showSubmenu2;
  }

  if (showSubmenu2) {

     if (ImGui::TreeNode("Batteriemessung")) {
      ImGui::PushStyleColor(
            ImGuiCol_Text,
            ImVec4(100/ 255.0f, 100/ 255.0f, 100 / 255.0f, 100 / 100.0f)); 

      ImGui::MenuItem("Analysiere Daten");

      ImGui::PopStyleColor(); 

      if (ImGui::MenuItem("Generiere Trainingsdaten"))
        open_generate_training_data = true;

      ImGui::TreePop();
    }
  }
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
  }

  /*if (ImGui::Button(load_json<std::string>(language, "menubar", "menu", "reset")
                        .c_str())) {
    sampler.reset();
    devices.clear();
    deviceManager.clearDevices();
    captureData.clear();
    flagPaused = true;
  }*/

  static bool showSubmenu1 = false;
  // First Menupoint shown as a button

  my_image_width = 0;
   my_image_height = 0;
   my_image_texture = 0;

  ret = LoadTextureFromFile("../images/SettingsWhite.png", &my_image_texture, &my_image_width, &my_image_height);

  IM_ASSERT(ret);

  if (ImGui::ImageButton("Settings",(void *)(intptr_t)my_image_texture,
               ImVec2(my_image_width *0.75  , my_image_height *0.75 ))) {
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

  static bool showSubmenu3 = false;
  // First Menupoint shown as a button

  my_image_width = 0;
   my_image_height = 0;
   my_image_texture = 0;

  ret = LoadTextureFromFile("../images/HelpWhite.png", &my_image_texture, &my_image_width, &my_image_height);

  if (ImGui::ImageButton("Help", (void *)(intptr_t)my_image_texture,
               ImVec2(my_image_width *0.75 , my_image_height *0.75 ))) {
    // Aktion bei Klick auf Menüpunkt 1
    showSubmenu3 = !showSubmenu3;
  }

  if (showSubmenu3) {
    if (ImGui::MenuItem(load_json<std::string>(language, "helplink").c_str())) {
      system(("start " + load_json<std::string>(config, "helplink")).c_str());
    }
  }

  ImGui::SetCursorPosY(ImGui::GetIO().DisplaySize.y * 0.95f);
  ImGui::Text(
      fmt::format("   Version: {}", CMakeGitVersion::VersionWithGit).c_str());

  mainMenuBarSize = ImGui::GetItemRectSize();
  ImGui::EndChild();
}