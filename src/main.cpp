#include <algorithm>
#include <cmake_git_version/version.hpp>
#include <set>

#include "VCDS_data.hpp"
#include "imagesHeaderToolbar.hpp"
#include "popups.hpp"
#include "settingspopup.hpp"
#include "style.hpp"
#include <cstdlib> // Für std::system



    

    void OpenURLInBrowser(const std::string& url) {
    // Plattformabhängiger Code zum Öffnen einer URL im Standardbrowser
    #ifdef _WIN32
        HINSTANCE result = ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
        if ((int)result <= 32) {
            std::cerr << "Fehler beim Öffnen der URL: " << (int)result << std::endl;
        }
    #elif __APPLE__
        std::string command = "open " + url;
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cerr << "Fehler beim Öffnen der URL: " << result << std::endl;
        }
    #elif __linux__
        std::string command = "xdg-open " + url;
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cerr << "Fehler beim Öffnen der URL: " << result << std::endl;
        }
    #else
        std::cerr << "Plattform nicht unterstützt!" << std::endl;
    #endif
    }



int main() {
    const std::string configpath = "config/config.json";
    set_config(configpath);
    nlohmann::json config = load_json_file(configpath);
    set_json(config);
    nlohmann::json language =
        load_json_file(load_json<std::string>(config, "languagepath") +
                       load_json<std::string>(config, "language") + ".json");
    // local variables
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_time_t);
    double xmax_paused{0};
    bool open_settings = false;
    bool open_generate_training_data = false;
    static bool flagPaused = true;
    bool flagDataNotSaved = true;
    bool Development = false;
    bool flagInitState = true;
    bool open_VCDS = false;
    bool upload_success = false;


    // main loop
    auto render = [&]() {
        if (flagInitState) {
            set_inital_config(config);
            flagInitState = false;
        }
        SetupImGuiStyle(false, 0.99f, config);
        ImGui::SetNextWindowPos({0.f, 0.f});
        auto windowSize{ImGui::GetIO().DisplaySize};
        ImGui::SetNextWindowSize(windowSize);
        const ImVec2 toolBtnSize{windowSize.x * .1f,
                                 windowSize.y * .1f};  // toolbar buttons size
        ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoTitleBar);

        if (Development && ImGui::Button("Development"))
            ImGui::OpenPopup("Development Colors");

        // Popup-Window content
        if (ImGui::BeginPopup("Development Colors")) {
            PopupStyleEditor();
            ImGui::EndPopup();
        }

        ImGui::BeginChild("Left Side", {windowSize.x * .18f, 0.f});
        set_side_menu(config, flagPaused, open_settings,
                      open_generate_training_data, open_VCDS);
        // there're four "BeginChild"s, one as the left side
        // and three on the right side
        ImGui::EndChild();  // end child "Left Side"
        ImGui::SameLine();
        ImGui::BeginChild("Right Side", {0.f, 0.f});
        if (sampler.has_value() && !flagPaused) sampler->copyOut(captureData);
        ImGui::BeginChild("Buttonstripe", {-1.f, windowSize.y * .1f}, false,
                          ImGuiWindowFlags_NoScrollbar);
        // ############################ Popup Save
        if (ImGui::BeginPopupModal(appLanguage[Key::Save_Recorded_Data],
                                   nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetItemDefaultFocus();
            saves_popup(config, language, captureData, now, now_time_t, now_tm,
                        flagDataNotSaved);
            ImGui::EndPopup();
        }
        // ############################ Popup Reset
        if (ImGui::BeginPopupModal(appLanguage[Key::Reset_q], nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetItemDefaultFocus();
            ImGui::Text(appLanguage[Key::Measure_not_saved]);
            if (ImGui::Button(appLanguage[Key::Continue_del])) {
                rstSettings();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button(appLanguage[Key::Back]))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // Initializing all variables for images in the toolbar
        static constexpr size_t size{4};  // number of pictures
        int PngRenderedCnt = 0;
        static bool loaded_png[size]{};
        static int image_height[size];
        static int image_width[size];
        static GLuint image_texture[size];
        static constexpr float iconsSacle{0.8f};

        // The order matters because of the counter for the images !!!
        static const unsigned char *imagesNames[] = {
            PlayButton_png, PauseButton_png, SaveButton_png, ResetButton_png};
        static const unsigned int imagesLen[] = {
            PlayButton_png_len, PauseButton_png_len, SaveButton_png_len,
            ResetButton_png_len};
        // Load the images for the SideBarMenu
        for (int i = 0; i < size; i++)
            if (!loaded_png[i]) {
                if (LoadTextureFromHeader(imagesNames[i], imagesLen[i],
                                          &image_texture[i], &image_width[i],
                                          &image_height[i]))
                    loaded_png[i] = true;
                else
                    fmt::println("Error Loading Png #{}.", i);
            }

        // ImGui::SetCursorPosY(windowSize.y * 0.05f);

        if (flagPaused) {
            // ######################## Buttonstripe
            if (!devices.empty())
                if (!sampler.has_value()) {
                    PngRenderedCnt = 0;
                    set_button_style_to(config, "start");  // Start Button
                    if (ImGui::ImageButton(
                            appLanguage[Key::Start],
                            (void *)(intptr_t)image_texture[PngRenderedCnt],
                            ImVec2(
                                image_width[PngRenderedCnt] * iconsSacle,
                                image_height[PngRenderedCnt] * iconsSacle))) {
                        sampler.emplace(deviceManager, std::move(devices));
                        flagPaused = false;
                        flagDataNotSaved = true;
                        for (auto &device : devices) {
                            device->send(Omniscope::Start{});
                        }
                    }
                    ImGui::PopStyleColor(3);
                }
            // set_button_style_to(config, "standart");
        } else {
            // ############################ Stop Button
            PngRenderedCnt = 1;
            set_button_style_to(config, "stop");
            if (ImGui::ImageButton(
                    appLanguage[Key::Stop],
                    (void *)(intptr_t)image_texture[PngRenderedCnt],
                    ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                           image_height[PngRenderedCnt] * iconsSacle))) {
                flagPaused = true;
                for (auto &device : sampler->sampleDevices) {
                    device.first->send(Omniscope::Stop{});
                }
            }
            ImGui::PopStyleColor(3);
        }
        if (flagPaused) {
            // Start/reset the measurement when the measurement is paused,
            // followed by a query as to whether the old data should be saved
            if (sampler.has_value()) {
                ImGui::SameLine();
                PngRenderedCnt = 0;
                set_button_style_to(config, "start");
                if (ImGui::ImageButton(
                        appLanguage[Key::Continue],
                        (void *)(intptr_t)image_texture[PngRenderedCnt],
                        ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                               image_height[PngRenderedCnt] * iconsSacle))) {
                    flagPaused = false;
                    flagDataNotSaved = true;
                    for (auto &device : sampler->sampleDevices) {
                        device.first->send(Omniscope::Start{});
                    }
                }
                ImGui::PopStyleColor(3);
                ImGui::SameLine();
                PngRenderedCnt = 3;

                set_button_style_to(config, "stop");
                if (ImGui::ImageButton(
                        appLanguage[Key::Reset],
                        (void *)(intptr_t)image_texture[PngRenderedCnt],
                        ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                               image_height[PngRenderedCnt] * iconsSacle))) {
                    if (flagDataNotSaved) {
                        ImGui::OpenPopup(appLanguage[Key::Reset_q]);
                        for (auto &device : sampler->sampleDevices) {
                            device.first->send(Omniscope::Stop{});
                        }
                    } else {
                        rstSettings();
                        flagPaused = true;
                    }
                }
                ImGui::PopStyleColor(3);
      }
      ImGui::SameLine();

                // gray out "Save" button when pop-up is open
                const bool pushStyle =
                    ImGui::IsPopupOpen(appLanguage[Key::Save_Recorded_Data]);

                if (pushStyle)
                    ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
                PngRenderedCnt = 2;
                if (ImGui::ImageButton(
                        appLanguage[Key::Save],
                        (void *)(intptr_t)image_texture[PngRenderedCnt],
                        ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                               image_height[PngRenderedCnt] * iconsSacle))) {
                    if (sampler.has_value())
                        ImGui::OpenPopup(appLanguage[Key::Save_Recorded_Data]);
                    else
                        ImGui::OpenPopup(
                            appLanguage[Key::Save_warning],
                            ImGuiPopupFlags_NoOpenOverExistingPopup);
                }
                info_popup(appLanguage[Key::Save_warning],
                           appLanguage[Key::No_dvc_available]);

                if (pushStyle) {
                    ImGui::PopStyleColor();
                }
            } else {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
                PngRenderedCnt = 2;
                ImGui::ImageButton(
                    appLanguage[Key::Save],
                    (void *)(intptr_t)image_texture[PngRenderedCnt],
                    ImVec2(image_width[PngRenderedCnt] * iconsSacle,
                           image_height[PngRenderedCnt] * iconsSacle));
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();  // end child "Buttonstripe"
            // ############################ Settings Menu
            static int title = 0;
            static std::vector<std::string> titles(2);  // two languages
            if (open_settings) {
                const auto EngItr = englishLan.find(Key::Settings);
                const auto GrmItr = germanLan.find(Key::Settings);
                // check returned value from find() and set titles
                if (EngItr != englishLan.end() && GrmItr != germanLan.end()) {
                    titles[0] = (std::string)EngItr->second + "###ID";
                    titles[1] = (std::string)GrmItr->second + "###ID";
                    ImGui::OpenPopup(titles[title].c_str());
                } else
                    fmt::println("Settings values not found.");
                open_settings = false;
            }
            if (ImGui::BeginPopupModal(titles[title].c_str(), nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::SetItemDefaultFocus();
                popup_settings(config, configpath, title);
                ImGui::EndPopup();
            }
            // Generate training data popup
            if (open_generate_training_data)
                generateTrainingData(open_generate_training_data, captureData,
                                     savedFileNames, config);

            // ############################### VCDS Menu
            if (open_VCDS) {
                // ImGui::OpenPopup("Generiere Trainingsdaten");
                // Hier wird eine Logik implementiert um die AW4.0 HUB-Oberfläche im Browser zu öffnen.
                    // Pfad zu deinem Python-Skript
                const char* script_path = "../src/script.py";
                
                // Befehl zum Ausführen des Python-Skripts
                std::string command = "python3 ";
                command += script_path;

                // Ausführen des Befehls mit system()
                int result = system(command.c_str());

                // Überprüfen des Rückgabewerts
                if (result == 0) {
                    std::cout << "Python script executed successfully." << std::endl;
                } else {
                    std::cerr << "Error executing Python script." << std::endl;
                }

                OpenURLInBrowser("https://bo-i-t.selfhost.eu/aw40-hub-test/demo/ui");


                open_VCDS = false;
            }
            if (ImGui::BeginPopupModal("Generiere Trainingsdaten", nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::SetItemDefaultFocus();
                popup_create_training_data_select(config, upload_success);
                ImGui::EndPopup();
            }

            // ############################ addPlots("Recording the data", ...)
            ImGui::Dummy({0.f, windowSize.y * .01f});
            PushPlotRegionColors();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,
                                windowSize.x * .009f);
            ImGui::BeginChild("Record Data", {0.f, windowSize.y * 0.62f},
                              ImGuiChildFlags_Border);

            // Axes 1 to 3
            // Check if time base for axes are same
            // check if egu and timescale for plot are same
            // error if third device is added
            addPlots("Recording the data", flagPaused,
                     [](double x_max, std::string yLabel, ImAxis_ axis,
                        double yMin, double yMax) {
                         ImPlot::SetupLegend(ImPlotLocation_NorthEast |
                                             ImPlotLegendFlags_Outside);
                         auto auxFlagsMeasuring = ImPlotAxisFlags_AutoFit |
                                                  ImPlotAxisFlags_NoGridLines;
                         auto auxFlagsPaused = ImPlotAxisFlags_NoGridLines;
                         ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22,
                                                nullptr, true);

                         if (!flagPaused) {
                             ImPlot::SetupAxis(axis, yLabel.c_str(),
                                               ImPlotAxisFlags_AutoFit);
                             ImPlot::SetupAxis(ImAxis_X1, "time [s]",
                                               ImPlotAxisFlags_AutoFit);
                             ImPlot::SetupAxisLimits(axis, yMin - 2, yMax + 2,
                                                     ImGuiCond_Always);
                             ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 1,
                                                     x_max + 9,
                                                     ImGuiCond_Always);

                         } else {
                             ImPlot::SetupAxis(ImAxis_X1, "time [s]");
                             ImPlot::SetupAxis(axis, yLabel.c_str());
                             ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10);
                             ImPlot::SetupAxisLimits(axis, yMin - 2, yMax + 2);
                         }
                     });

            ImGui::EndChild();  // end child Record Data
            ImGui::PopStyleVar();
            PopPlotRegionColors();
            // ############################ Devicelist
            SetDeviceMenuStyle();

            ImGui::Dummy({0.f, windowSize.y * .01f});
            ImGui::BeginChild("Devicelist");
            ImGui::Dummy({windowSize.x * .36f, 0.f});
            ImGui::SameLine();
            ImGui::Text(appLanguage[Key::Devices_found]);
            devicesList(flagPaused);
            ImGui::EndChild();  // end child "Devicelist"
            ImGui::EndChild();  // end child "Right Side"
            ImGui::End();
        };
    ImGuiInstance window{1500, 800,
                        fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                    CMakeGitVersion::Project::Version)};
    while (window.run(render)){};

    return 0;
}
