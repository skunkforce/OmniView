#include <algorithm>
#include <cmake_git_version/version.hpp>
#include <set>

#include "popups.hpp"
#include "settingspopup.hpp"
#include "style.hpp"
std::vector<std::string> uniqueSorted(const std::vector<std::string>& input) {
    std::set<std::string> uniqueSet(input.begin(), input.end());

    std::vector<std::string> result(uniqueSet.begin(), uniqueSet.end());

    return result;
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
    bool open_settings = false;
    bool open_generate_training_data = false;
    bool upload_success = false;
    static bool flagPaused = true;
    bool flagDataNotSaved = true;
    bool Development = false;

    // main loop
    auto render = [&]() {
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
                      open_generate_training_data);
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

        if (flagPaused) {
            // ######################## Buttonstripe
            if (!devices.empty())
                if (!sampler.has_value()) {
                    set_button_style_to(config, "start");  // Start Button
                    if (ImGui::Button(appLanguage[Key::Start], toolBtnSize)) {
                        sampler.emplace(deviceManager, std::move(devices));
                        flagPaused = false;
                        flagDataNotSaved = true;
                    }
                    ImGui::PopStyleColor(3);
                }
            // set_button_style_to(config, "standart");
        } else {
            // ############################ Stop Button
            set_button_style_to(config, "stop");
            if (ImGui::Button(appLanguage[Key::Stop], toolBtnSize)) {
                flagPaused = true;
                for (auto& device : sampler->sampleDevices) {
                    device.first->send(Omniscope::Stop{});
                }
            }
            ImGui::PopStyleColor(3);
        }
        if (flagPaused) {
            ImGui::SameLine();

            // Start/reset the measurement when the measurement is paused,
            // followed by a query as to whether the old data should be saved
            if (sampler.has_value()) {
                ImGui::SameLine();
                set_button_style_to(config, "start");
                if (ImGui::Button(appLanguage[Key::Continue], toolBtnSize)) {
                    flagPaused = false;
                    flagDataNotSaved = true;
                    for (auto& device : sampler->sampleDevices) {
                        device.first->send(Omniscope::Start{});
                    }
                }
                ImGui::PopStyleColor(3);
                ImGui::SameLine();

                set_button_style_to(config, "stop");
                if (ImGui::Button(appLanguage[Key::Reset], toolBtnSize)) {
                    if (flagDataNotSaved)
                        ImGui::OpenPopup(appLanguage[Key::Reset_q]);
                    else {
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

            if (pushStyle) ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
            if (ImGui::Button(appLanguage[Key::Save], toolBtnSize)) {
                if (sampler.has_value())
                    ImGui::OpenPopup(appLanguage[Key::Save_Recorded_Data]);
                else
                    ImGui::OpenPopup(appLanguage[Key::Save_warning],
                                     ImGuiPopupFlags_NoOpenOverExistingPopup);
            }
            info_popup(appLanguage[Key::Save_warning],
                       appLanguage[Key::No_dvc_available]);

            if (pushStyle) ImGui::PopStyleColor();
        } else {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
            ImGui::Button(appLanguage[Key::Save], toolBtnSize);
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();  // end child "Buttonstripe"
        // ############################ Settings Menu
        std::string settingstitle =
            load_json<std::string>(language, "settings", "title");
        if (open_settings) {
            ImGui::OpenPopup(settingstitle.c_str());
            open_settings = false;
        }
        if (ImGui::BeginPopupModal(settingstitle.c_str(), nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetItemDefaultFocus();
            popup_settings(config, language, configpath);
            ImGui::EndPopup();
        }
        // Generate training data popup
        if (open_generate_training_data)
            generateTrainingData(open_generate_training_data, captureData,
                                 savedFileNames, config);

        // ############################ addPlots("Recording the data", ...)
        ImGui::Dummy({0.f, windowSize.y * .01f});
        PushPlotRegionColors();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,
                            windowSize.x * .009f);
        ImGui::BeginChild("Record Data", {0.f, windowSize.y * 0.62f},
                          ImGuiChildFlags_Border);
        auto const plotRegion = ImGui::GetContentRegionAvail();

        // Axes 1 to 3
        // Check if time base for axes are same
        // check if egu and timescale for plot are same
        // error if third device is added
        addPlots("Recording the data", flagPaused, [](double x_max) {
            ImPlot::SetupLegend(ImPlotLocation_NorthEast |
                                ImPlotLegendFlags_Outside);
            // ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22, nullptr,
            // true);
            static std::vector<std::string> egus;
            static std::vector<std::string> x_labels;
            static std::vector<std::string> y_labels;
            static bool init = true;
            if (sampler.has_value()) {
                if (init) {
                    std::size_t numberOfDevices = sampler->sampleDevices.size();
                    for (auto& device : sampler->sampleDevices) {
                        if (device.first->getEgu().has_value()) {
                            auto egu = device.first->getEgu().value();
                            fmt::print("egu found: {}\n", egu);
                            egus.push_back(fmt::format(
                                "{} {}", appLanguage[Key::Voltage], egu));
                        }
                    }
                    egus = uniqueSorted(egus);
                    fmt::print("egus: {}\n", egus);
                    init = false;
                }
                if(!egus.empty() && !flagPaused){
                    //Setup Axes for corresponding EGUs found
                    if(egus.size() >= 1){
                        //Setup First Axis with first text from one EGU
                    }
                    if(egus.size() >= 2){
                        //Setup second Axis with third text from one EGU
                    }
                    if(egus.size() >= 3){
                        //Setup third Axis with third text from one EGU
                    }
                }else if(!egus.empty() && flagPaused){
                    //Same as above but with paused axis
                    if(egus.size() >= 1){
                        //Setup First Axis with first text from one EGU
                    }
                    if(egus.size() >= 2){
                        //Setup second Axis with third text from one EGU
                    }
                    if(egus.size() >= 3){
                        //Setup third Axis with third text from one EGU
                    }
                }else if(egus.empty() && !flagPaused){
                    //Setup first Axis for default Text Measuring
                }else if(egus.empty() && flagPaused){
                    //Setup first Axis for default Text Paused
                }
            }
            auto auxFlagsMeasuring =
                ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoGridLines;
            auto auxFlagsPaused = ImPlotAxisFlags_NoGridLines;
            /*
                        if (!flagPaused) {
                            if (numberOfDevices >= 1) {
                                ImPlot::SetupAxis(ImAxis_X1,
               x_labels[0].c_str(), ImPlotAxisFlags_AutoFit);
                                ImPlot::SetupAxis(ImAxis_Y1,
               y_labels[0].c_str(), ImPlotAxisFlags_AutoFit);
                                ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 1,
               x_max + 2, ImGuiCond_Always);
                            }
                            if (numberOfDevices >= 2) {
                                ImPlot::SetupAxis(ImAxis_X2,
               x_labels[1].c_str(), auxFlagsMeasuring);
                                ImPlot::SetupAxis(ImAxis_Y2,
               y_labels[1].c_str(), auxFlagsMeasuring);
                                ImPlot::SetupAxisLimits(ImAxis_X2, x_max - 1,
               x_max + 2, ImGuiCond_Always);
                            }
                            if (numberOfDevices >= 3) {
                                ImPlot::SetupAxis(ImAxis_X3,
               x_labels[2].c_str(), auxFlagsMeasuring);
                                ImPlot::SetupAxis(ImAxis_Y3,
               y_labels[2].c_str(), auxFlagsMeasuring);
                                ImPlot::SetupAxisLimits(ImAxis_X3, x_max - 1,
               x_max + 2, ImGuiCond_Always);
                            }
                        } else {
                            ImPlot::SetupAxis(ImAxis_X1, x_labels[0].c_str());
                            ImPlot::SetupAxis(ImAxis_Y1, y_labels[0].c_str());
                            ImPlot::SetupAxesLimits(0, 10, -10, 200);
                            if (numberOfDevices >= 2) {
                                ImPlot::SetupAxis(ImAxis_X2,
               x_labels[1].c_str(), auxFlagsPaused);
                                ImPlot::SetupAxis(ImAxis_Y2,
               y_labels[1].c_str(), auxFlagsPaused);
                                ImPlot::SetupAxisLimits(ImAxis_X2, 0, 10);
                                ImPlot::SetupAxisLimits(ImAxis_Y2, -10, 200);
                            }
                            if (numberOfDevices >= 3) {
                                ImPlot::SetupAxis(ImAxis_X3,
               x_labels[2].c_str(), auxFlagsPaused);
                                ImPlot::SetupAxis(ImAxis_Y3,
               y_labels[2].c_str(), auxFlagsPaused);
                                ImPlot::SetupAxisLimits(ImAxis_X3, 0, 10);
                                ImPlot::SetupAxisLimits(ImAxis_Y3, -10, 200);
                            }
                        }
                */
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
    while (window.run(render))
        ;
    return 0;
}
