
#include "../jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../../images/AutoInternLogo_data.h"
#include "../../images/DiagnosticsWhite_data.h"
#include "../../images/HelpWhite_data.h"
#include "../../images/SearchDevicesWhite_data.h"
#include "../../images/SettingsWhite_data.h"

#include "../languages.hpp"
#include <iostream>
#include <string>

// Function to set the SideBarMenu in the main.cpp // first version

namespace SideBarRegion {

    void SetSideBarMenu(
        nlohmann::json& language, std::vector<std::string>& availableLanguages,
        nlohmann::json& config, const std::string& configpath, bool& open_settings,
        std::optional<OmniscopeSampler>& sampler,
        std::vector<std::shared_ptr<OmniscopeDevice>>& devices,
        OmniscopeDeviceManager& deviceManager,
        std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        & captureData,
        bool& flagPaused, bool& open_generate_training_data,
        ImVec2& mainMenuBarSize,
        std::map<Omniscope::Id, std::array<float, 3>>& colorMap) {

        // Initializing all variables for images
        static constexpr int size = 5;         // number of pictures
        static int counterPngRendered = 0; // counter for the number of pictures rendered
        static bool loaded_png[size] = {};
        static int my_image_height[size];
        static int my_image_width[size];
        static GLuint my_image_texture[size];

        // The order matters because of the counter for the images !!!
        const unsigned char* imagesNames[size] = {
            AutoInternLogo_png, SearchDevicesWhite_png, DiagnosticsWhite_png,
            SettingsWhite_png, HelpWhite_png };
        unsigned int imagesLen[size] = {
            AutoInternLogo_png_len, SearchDevicesWhite_png_len,
            DiagnosticsWhite_png_len, SettingsWhite_png_len, HelpWhite_png_len };

        // Load the images for the SideBarMenu
        for (int w = 0; w < size; w++)
            if (!loaded_png[w]) {
                if (LoadTextureFromHeader(imagesNames[w], imagesLen[w],
                    &my_image_texture[w], &my_image_width[w],
                    &my_image_height[w]))
                    loaded_png[w] = true;
                else
                    fmt::println("Error Loading Png #{}.", w);
            }

        // Set the SideBarMenu Colors
        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_Border] = { 1.0f, 1.0f, 1.0f, 1.0f };
        style.Colors[ImGuiCol_BorderShadow] = { 37 / 255.0f, 1.0f, 43 / 255.0f, 1.0f };

        // InitDevices after searching for devices
        static constexpr int VID = 0x2e8au;
        static constexpr int PID = 0x000au;
        auto initDevices = [&]() {
            devices = deviceManager.getDevices(VID, PID);
            for (auto& device : devices) {
                auto id = device->getId().value();
                if (!colorMap.contains(id)) {
                    ImPlot::PushColormap(ImPlotColormap_Dark);
                    auto c = ImPlot::GetColormapColor((colorMap.size() % 7) + 1);
                    colorMap[id] = std::array<float, 3>{c.x, c.y, c.z};
                    ImPlot::PopColormap();
                }
                auto& color = colorMap[id];
                device->send(
                    Omniscope::SetRgb{ static_cast<std::uint8_t>(color[0] * 255),
                                      static_cast<std::uint8_t>(color[1] * 255),
                                      static_cast<std::uint8_t>(color[2] * 255) });
            }
            };

        // set the menu to the left side of the window ; important size in percentage!
        ImGui::SetCursorPos(ImVec2(0, 0));

        ////////////////////////////////////
        ////// Begin the SideBarMenu ///////
        ////////////////////////////////////
        auto windowSize{ ImGui::GetIO().DisplaySize };
        ImGui::BeginChild("SideBarMenu", { windowSize.x * 0.18f, windowSize.y },
            true);
        if (loaded_png[counterPngRendered]) {
            // render the AIGroupLogo
            ImGui::Image(
                (void*)(intptr_t)my_image_texture[counterPngRendered],
                ImVec2(my_image_width[counterPngRendered] * windowSize.x * 0.00035,
                    my_image_height[counterPngRendered] * windowSize.y * 0.0005));
            //ImGui::Text(" there's something here ");
            ++counterPngRendered;
        }

        static constexpr float iconsSacle{ 0.6f };
        // Start only if devices are available, otherwise search for devices
        if (loaded_png[counterPngRendered]) { // search for Devices
            if (!sampler.has_value()) {
                if (ImGui::ImageButton(
                    "Load new Devices",
                    (void*)(intptr_t)my_image_texture[counterPngRendered],
                    ImVec2(my_image_width[counterPngRendered] * iconsSacle,
                        my_image_height[counterPngRendered] * iconsSacle))) {
                    devices.clear();
                    deviceManager.clearDevices();
                    initDevices();
                }
            }
        }
        ++counterPngRendered;

        // Changing the Menustructure to a TreeNode Structure
        if (loaded_png[counterPngRendered]) { // Diagnostics
            static bool showSubmenu2 = false;
            if (ImGui::ImageButton(
                "Diagnostics",
                (void*)(intptr_t)my_image_texture[counterPngRendered],
                ImVec2(my_image_width[counterPngRendered] * iconsSacle,
                    my_image_height[counterPngRendered] * iconsSacle))) {
                // Aktion bei Klick auf Menüpunkt 1
                showSubmenu2 = !showSubmenu2;
            }
            ++counterPngRendered;

            if (showSubmenu2) {
                if (ImGui::TreeNode("Batteriemessung")) {
                    ImGui::PushStyleColor(
                        ImGuiCol_Text,
                        ImVec4(100 / 255.0f, 100 / 255.0f, 100 / 255.0f, 100 / 100.0f));

                    ImGui::MenuItem(appLanguage[Key::Anlyz_crnt_waveform]);
                    ImGui::PopStyleColor();

                    if (ImGui::MenuItem(appLanguage[Key::Gnrt_trning_data]))
                        open_generate_training_data = true;

                    ImGui::TreePop();
                }
            }
        }

        if (loaded_png[counterPngRendered]) { // Settings
            static bool showSubmenu1 = false;
            if (ImGui::ImageButton(
                "Settings", (void*)(intptr_t)my_image_texture[counterPngRendered],
                ImVec2(my_image_width[counterPngRendered] * iconsSacle,
                    my_image_height[counterPngRendered] * iconsSacle))) {
                // Aktion bei Klick auf Menüpunkt 1
                showSubmenu1 = !showSubmenu1;
            }
            ++counterPngRendered;

            // if button has been clicked to that :
            if (showSubmenu1) {
                if (ImGui::TreeNode(appLanguage[Key::LanOption])) {
                    for (const auto& lang : availableLanguages) {
                        if (ImGui::Button(lang.c_str())) {
                            config["language"] = lang;
                            write_json_file(configpath, config);
                            appLanguage = germanLan;
                        }
                        if (ImGui::MenuItem(appLanguage[Key::English]))
                            appLanguage = englishLan;
                    }
                    ImGui::TreePop();
                }
                if (ImGui::MenuItem(appLanguage[Key::Settings]))
                    open_settings = true;
            }
        }
        if (loaded_png[counterPngRendered]) { // Help
            static bool showSubmenu3 = false;
            // First Menupoint shown as a button

            if (ImGui::ImageButton(
                "Help", (void*)(intptr_t)my_image_texture[counterPngRendered],
                ImVec2(my_image_width[counterPngRendered] * iconsSacle,
                    my_image_height[counterPngRendered] * iconsSacle))) {
                // Aktion bei Klick auf Menüpunkt 1
                showSubmenu3 = !showSubmenu3;
            }
            ++counterPngRendered;

            if (showSubmenu3)
                if (ImGui::MenuItem(appLanguage[Key::HelpLink]))
                    system(("start " + load_json<std::string>(config, "helplink")).c_str());
        }

        ImGui::SetCursorPosY(ImGui::GetIO().DisplaySize.y * 0.95f);
        ImGui::Text(fmt::format("{}: {}", appLanguage[Key::Version],
            CMakeGitVersion::VersionWithGit)
            .c_str());

        mainMenuBarSize = ImGui::GetItemRectSize();
        ImGui::EndChild();
        counterPngRendered = 0;
    } // EndofFunction

} // namespace SideBarRegion

// Analyses that will be added in a later process :
/*
   if (ImGui::TreeNode(appLanguage["Compression"])) {
     ImGui::MenuItem(appLanguage["Anlyz_crnt_waveform"]);
     if (ImGui::MenuItem(appLanguage["Gnrt_trning_data"]))
       open_generate_training_data = true;

     ImGui::TreePop();
   }

   // greyed out color style
   // ImGui::PushStyleColor(
   //  ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

   if (ImGui::TreeNode(appLanguage["Timing-Belt"])) {
     ImGui::MenuItem(appLanguage["Anlyz_crnt_waveform"]);
     if (ImGui::MenuItem(appLanguage["Gnrt_trning_data"]))
       open_generate_training_data = true;
     ImGui::TreePop();
   }

   if (ImGui::TreeNode(appLanguage["Fuel-Delivery-Pump"])) {
     ImGui::MenuItem(appLanguage["Anlyz_crnt_waveform"]);
     if (ImGui::MenuItem(appLanguage["Gnrt_trning_data"]))
       open_generate_training_data = true;
     ImGui::TreePop();
   }
   if (ImGui::TreeNode(appLanguage["Common-Rail-Pressure"])) {
     ImGui::MenuItem(appLanguage["Anlyz_crnt_waveform"]);
     if (ImGui::MenuItem(appLanguage["Gnrt_trning_data"]))
       open_generate_training_data = true;
     ImGui::TreePop();
   }

   // ImGui::PopStyleColor(2);
   // ImGui::PushStyleColor(ImGuiCol_Text,
   // load_json<Color>(config, "text", "color", "normal"));
 } */