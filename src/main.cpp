// #include "OmniscopeCommunication.hpp"
// clang-format off
#include <boost/asio.hpp>
//clang-format on
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "apihandler.hpp"
#include "create_training_data.hpp"
#include "jasonhandler.hpp"
#include "settingspopup.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <algorithm>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <thread>
// clang-format off
#include <imfilebrowser.h>
// clang-format on

std::string configpath = "../config/config.json";

static std::vector<std::string> getAvailableLanguages(
    std::string const& languageFolder) {
    std::vector<std::string> languages;
    for (auto const& entry :
         std::filesystem::directory_iterator(languageFolder)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if (extension == ".json") {
                std::string filename = entry.path().filename().stem().string();
                languages.push_back(filename);
            }
        }
    }
    return languages;
}

static void save(
    std::map<Omniscope::Id, std::vector<std::pair<double, double>>> const&
        alignedData,
    std::filesystem::path const& outFile) {
    auto minSize = std::numeric_limits<std::size_t>::max();

    std::vector<std::vector<std::pair<double, double>> const*> data;

    std::string fileContent;

    for (auto sep = std::string_view{};
         auto const& [device, values] : alignedData) {
        fileContent += sep;
        fileContent += "\"";
        fileContent += fmt::format("{}-{}", device.type, device.serial);
        fileContent += "\"";
        sep = ",";
        minSize = std::min(values.size(), minSize);
        data.push_back(std::addressof(values));
    }
    fileContent += '\n';

    for (std::size_t i{}; i < minSize; ++i) {
        fileContent += fmt::format("{}", i);
        fileContent += ",";
        for (auto sep = std::string_view{}; auto const& vec : data) {
            auto const dataPair = (*vec)[i];
            fileContent += sep;
            if (dataPair.second) {
                fileContent += fmt::format("{}", dataPair.second);
            }
            sep = ",";
        }
        fileContent += '\n';
    }

    fileContent += '\n';

    auto path = outFile;
    path.remove_filename();
    if (!std::filesystem::exists(path) && !path.empty()) {
        std::filesystem::create_directories(path);
    }

    fmt::print("start save {}\n", outFile.string());
    std::ofstream file{outFile};
    // fmt::print("{}\n", fileContent);
    file << fileContent;
    file.flush();
    file.close();

    fmt::print("finished save\n");
}

static void set_button_style_to(nlohmann::json const& config,
                                std::string const& name) {
    ImGui::PushStyleColor(
        ImGuiCol_Button,
        ImVec4(load_json<Color>(config, "button", name, "normal")));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ImVec4(load_json<Color>(config, "button", name, "hover")));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        ImVec4(load_json<Color>(config, "button", name, "active")));
}

static void load_settings(nlohmann::json const& config) {
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg,
                          ImVec4(load_json<Color>(config, "menubar", "main")));
    ImGui::PushStyleColor(ImGuiCol_PopupBg,
                          ImVec4(load_json<Color>(config, "menubar", "popup")));
    ImGui::PushStyleColor(
        ImGuiCol_Text,
        ImVec4(load_json<Color>(config, "text", "color", "normal")));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          ImVec4(load_json<Color>(config, "window", "color")));
    set_button_style_to(config, "standart");

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = load_json<float>(config, "text", "scale");
}

// ###########################################################################
// ############# INT MAIN BEGINN #############################################
// ###########################################################################

int main() {
    nlohmann::json config;
    std::string addpath = "";

    if (std::filesystem::exists("../config/config.json")) {
        configpath = "../config/config.json";

    } else if (std::filesystem::exists("../../config/config.json")) {
        configpath = "../../config/config.json";
        addpath = "../";

    } else {
        fmt::print("did not find config.json");
        // close programm and with a message, no configfile found
        return 1;  // Rückgabewert 1 signalisiert einen Fehler
    }
    config = load_json_file(configpath);

    std::vector<std::string> availableLanguages = getAvailableLanguages(
        addpath + load_json<std::string>(config, ("languagepath")));

    nlohmann::json language = load_json_file(
        addpath + load_json<std::string>(config, "languagepath") +
        load_json<std::string>(config, "language") + ".json");

    static constexpr int VID = 0x2e8au;
    static constexpr int PID = 0x000au;
    // static constexpr std::size_t captureDataReserve = 1 << 26;
    OmniscopeDeviceManager deviceManager{};
    std::vector<std::shared_ptr<OmniscopeDevice>>
        devices;  // = deviceManager.getDevices(VID, PID);
    // auto newDevices = devices;

    //   auto startTimepoint = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_time_t);

    double xmax_paused{0};
    static bool open_settings = false;

    static ImVec2 mainMenuBarSize;
    std::optional<OmniscopeSampler> sampler{};
    std::map<Omniscope::Id, std::vector<std::pair<double, double>>> captureData;

    std::map<Omniscope::Id, std::array<float, 3>> colorMap;

    std::string path;
    path.resize(256);

    auto addPlots = [&, firstRun = std::set<std::string>{}](
                        auto const& name, auto const& plots,
                        auto axesSetup) mutable {
        auto const plotRegion = ImGui::GetContentRegionAvail();
        if (ImPlot::BeginPlot(name, plotRegion)) {
            double x_min = std::numeric_limits<double>::max();
            double x_max = std::numeric_limits<double>::min();

            for (auto const& plot : plots) {
                if (!plot.second.empty()) {
                    x_min = std::min(x_min, plot.second.front().first);
                    x_max = std::max(x_max, plot.second.back().first);
                }
            }

            axesSetup(x_min, x_max);

            auto const limits = [&]() {
                if (!firstRun.contains(name)) {
                    firstRun.insert(name);
                    return ImPlotRect(x_min, x_max, 0, 0);
                }
                return ImPlot::GetPlotLimits();
            }();

            auto addPlot = [&](auto const& plot) {
                if (!plot.second.empty()) {
                    auto const start = [&]() {
                        auto p = std::lower_bound(
                            plot.second.begin(), plot.second.end(),
                            std::pair<double, double>{limits.X.Min, 0});
                        if (p != plot.second.begin()) {
                            return p - 1;
                        }
                        return p;
                    }();

                    auto const end = [&]() {
                        auto p = std::upper_bound(
                            start, plot.second.end(),
                            std::pair<double, double>{limits.X.Max, 0});
                        if (p != plot.second.end()) {
                            return p + 1;
                        }
                        return p;
                    }();

                    std::size_t const stride = [&]() -> std::size_t {
                        auto const s =
                            std::distance(start, end) / (plotRegion.x * 2.0);
                        if (1 >= s) {
                            return 1;
                        }
                        return static_cast<std::size_t>(s);
                    }();

                    ImPlot::PlotLine(
                        fmt::format("{}-{}", plot.first.type, plot.first.serial)
                            .c_str(),
                        std::addressof(start->first),
                        std::addressof(start->second),
                        static_cast<std::size_t>(std::distance(start, end)) /
                            stride,
                        0, 0, 2 * sizeof(double) * stride);
                }
            };

            for (auto const& plot : plots) {
                ImPlot::SetNextLineStyle(ImVec4{colorMap[plot.first][0],
                                                colorMap[plot.first][1],
                                                colorMap[plot.first][2], 1.0f});
                addPlot(plot);
            }

            ImPlot::EndPlot();
        }
    };

    auto render = [&]() {
        load_settings(config);
        ImGui::SetNextWindowPos(ImVec2(0.0f, mainMenuBarSize.y));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove);  //

        // ############################ Menu bar ##############################
        //  main menu
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu(
                load_json<std::string>(language, "menubar", "menu", "label")
                    .c_str())) {
            if (ImGui::BeginMenu(load_json<std::string>(language, "menubar",
                                                        "menu",
                                                        "language_option")
                                     .c_str())) {
                for (const auto& lang : availableLanguages) {
                    if (ImGui::MenuItem(lang.c_str())) {
                        config["language"] = lang;
                        write_json_file(configpath, config);
                    }
                }

                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(load_json<std::string>(language, "menubar",
                                                       "menu", "settings")
                                    .c_str())) {
                open_settings = true;
            }
            if (ImGui::MenuItem(load_json<std::string>(language, "menubar",
                                                       "menu", "clearcapture")
                                    .c_str())) {
                captureData.clear();
            }
            if (ImGui::MenuItem(
                    load_json<std::string>(language, "menubar", "menu", "reset")
                        .c_str())) {
                sampler.reset();
                captureData.clear();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(
                load_json<std::string>(language, "menubar", "view", "label")
                    .c_str())) {
            ImGui::MenuItem(
                load_json<std::string>(language, "menubar", "view", "first")
                    .c_str());
            ImGui::MenuItem(
                load_json<std::string>(language, "menubar", "view", "second")
                    .c_str());
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(
                load_json<std::string>(language, "menubar", "help", "label")
                    .c_str())) {
            ImGui::MenuItem(
                load_json<std::string>(language, "menubar", "help", "first")
                    .c_str());
            ImGui::MenuItem(
                load_json<std::string>(language, "menubar", "help", "second")
                    .c_str());
            ImGui::EndMenu();
        }
        mainMenuBarSize = ImGui::GetItemRectSize();
        ImGui::EndMainMenuBar();

        // ############################ Live Capture
        // ##############################
        ImGui::BeginChild("Live Capture", ImVec2(-1, 620));
        if (sampler.has_value()) {
            sampler->copyOut(captureData);
        }

        addPlots(
            "Aufnahme der Daten", captureData,
            [&sampler, &xmax_paused](auto /*x_min*/, auto x_max) {
                if (sampler.has_value()) {
                    ImPlot::SetupAxes("x [Datenpunkte]", "y [ADC Wert]",
                                      ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 7500,
                                            x_max + 7500, ImGuiCond_Always);
                } else {
                    xmax_paused = x_max;
                    ImPlot::SetupAxes("x [Datenpunkte]", "y [ADC Wert]", 0, 0);
                }
            });

        ImGui::EndChild();

        ImGui::BeginChild("Buttonstripe",
                          ImVec2(-1, ImGui::GetTextLineHeightWithSpacing()),
                          false, ImGuiWindowFlags_NoScrollbar);

        // ############################ Popup Speichern
        // ##############################
        if (ImGui::BeginPopupModal("Speichern der aufgenommenen Daten", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetItemDefaultFocus();

            static char inputvin[18] = "";
            static char mileage[10] = "";
            static char scantype[255] = "";
            ImGui::SetItemDefaultFocus();
            ImGui::InputText(
                load_json<std::string>(language, "input", "fin", "label")
                    .c_str(),
                inputvin, sizeof(inputvin));
            ImGui::InputText(
                load_json<std::string>(language, "input", "mileage", "label")
                    .c_str(),
                mileage, sizeof(mileage));
            ImGui::InputText(
                load_json<std::string>(language, "input", "scantype", "label")
                    .c_str(),
                scantype, sizeof(scantype));

            if (ImGui::Button(
                    load_json<std::string>(language, "button", "save").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
                now = std::chrono::system_clock::now();

                now_time_t = std::chrono::system_clock::to_time_t(now);
                now_tm = *std::gmtime(&now_time_t);

                std::string_view path_sv{path.data()};
                std::string filename{
                    fmt::format("{}-{:%Y-%m-%dT%H-%M}.csv", mileage, now)};
                std::filesystem::path path_path = path_sv;
                if (captureData.empty()) {
                    ImGui::CloseCurrentPopup();
                } else {
                    // create the given folder_structure
                    std::filesystem::path first_folder =
                        load_json<std::filesystem::path>(config, "scanfolder");
                    std::filesystem::path complete_path =
                        first_folder / inputvin / scantype;
                    std::filesystem::create_directories(complete_path);

                    save(captureData, path_path / complete_path / filename);

                    // nicht mehr im save-kontext, sondern in create training
                    // data send_to_api(config, path_path / filename, inputvin,
                    // scantype);
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(
                    load_json<std::string>(language, "button", "back").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        ImGui::SetNextWindowPos(ImVec2(0, 100));
        ImGui::SetNextWindowSize(ImVec2(0, 800));
        if (!sampler.has_value()) {
            if (ImGui::BeginPopupModal("Erstellung Lerndatensatz", nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoMove)) {
                ImGui::SetItemDefaultFocus();

                popup_create_training_data_select(config, language);
                ImGui::EndPopup();
            }
            // Start nur wenn Devices vorhanden soind, sonst Suche Geräte
            if (ImGui::Button("Suche Geräte",
                              ImVec2(load_json<Size>(config, "button")))) {
                devices.clear();
                deviceManager.clearDevices();
                devices = deviceManager.getDevices(VID, PID);

                for (auto& device : devices) {
                    auto id = device->getId().value();
                    if (!colorMap.contains(id)) {
                        ImPlot::PushColormap(ImPlotColormap_Dark);
                        auto c = ImPlot::GetColormapColor(colorMap.size());
                        colorMap[id] = std::array<float, 3>{c.x, c.y, c.z};
                        ImPlot::PopColormap();
                    }

                    auto& color = colorMap[id];
                    fmt::print("col {}\n", color);
                    device->send(Omniscope::SetRgb{
                        static_cast<std::uint8_t>(color[0] * 255),
                        static_cast<std::uint8_t>(color[1] * 255),
                        static_cast<std::uint8_t>(color[2] * 255)});
                }
            }

            if (!devices.empty()) {
                ImGui::SameLine();
                // ############################ Start Button
                // ##############################
                set_button_style_to(config, "start");
                if (ImGui::Button("Start",
                                  ImVec2(load_json<Size>(config, "button")))) {
                    // sampler anlegen
                    // std::optinal<OmniscopeSampler> sampler{};
                    sampler.emplace(deviceManager, std::move(devices));
                }
                ImGui::PopStyleColor(3);
            }
            // set_button_style_to(config, "standart");
        } else {
            // ############################ Stop Button
            // ##############################
            set_button_style_to(config, "stop");
            if (ImGui::Button("Stop",
                              ImVec2(load_json<Size>(config, "button")))) {
                sampler.reset();
            }
            ImGui::PopStyleColor(3);
            // set_button_style_to(config, "standart");
        }
        if (!sampler.has_value()) {
            ImGui::SameLine();
            if (ImGui::Button("Speichern",
                              ImVec2(load_json<Size>(config, "button")))) {
                // savecontext = true;//Opens new overlay

                ImGui::OpenPopup("Speichern der aufgenommenen Daten");
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                load_json<Color>(config, "text", "color", "inactive"));
            // ImGui::FileBrowser fileBrowser;
            if (ImGui::Button("Analysiere Daten",
                              ImVec2(load_json<Size>(config, "button")))) {
                // fileBrowser.Open();
            }
            /*
            //fileBrowser.Display();
            if (fileBrowser.HasSelected()) {
                // Hier kannst du auf die ausgewählten Dateien zugreifen
                for (const auto &selectedFile : fileBrowser.GetSelected()) {
                    std::string filename = selectedFile.string();
                    // Verarbeite die ausgewählte Datei
                    // ...
                }

                fileBrowser.ClearSelected();
            }
            */
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                load_json<Color>(config, "text", "color", "normal"));
            ImGui::SameLine();

            // ############################ Button create trainings data
            // ##############################
            if (ImGui::Button("Erstelle Lerndatensatz",
                              ImVec2(load_json<Size>(config, "button")))) {
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(ImVec2(0, 0));
                ImGui::OpenPopup("Erstellung Lerndatensatz");
            }
            ImGui::PopStyleColor();
        } else {
            ImGui::SameLine();
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                load_json<Color>(config, "text", "color", "inactive"));
            if (ImGui::Button("Speichern",
                              ImVec2(load_json<Size>(config, "button")))) {
            }
            ImGui::SameLine();
            if (ImGui::Button("Analysiere Daten",
                              ImVec2(load_json<Size>(config, "button")))) {
            }
            ImGui::SameLine();
            if (ImGui::Button("Erstelle Lerndatensatz",
                              ImVec2(load_json<Size>(config, "button")))) {
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        // ############################ Settings Menu
        // ##############################
        std::string settingstitle =
            load_json<std::string>(language, "settings", "title");
        if (open_settings == true) {
            ImGui::OpenPopup(settingstitle.c_str());
            open_settings = false;
        }
        if (ImGui::BeginPopupModal(settingstitle.c_str(), nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetItemDefaultFocus();
            popup_settings(config, language, configpath);
            ImGui::EndPopup();
        }

        // ############################ Devicelist
        // ##############################
        ImGui::BeginChild("Devicelist", ImVec2(-1, 300));
        // ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        // ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
        //                       ImVec2(0.5f, 0.5f));

        ImGui::Text("gefundene Geräte:");
        if (ImGui::BeginListBox("##", ImVec2(1024, -1))) {
            for (auto& device : devices) {
                auto& color = colorMap[device->getId().value()];
                if (ImGui::ColorEdit3(
                        fmt::format(
                            "{:<32}",
                            fmt::format("{}-{}", device->getId().value().type,
                                        device->getId().value().serial))
                            .c_str(),
                        color.data(),
                            ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_NoPicker |
                            ImGuiColorEditFlags_NoTooltip)) {
                    device->send(Omniscope::SetRgb{
                        static_cast<std::uint8_t>(color[0] * 255),
                        static_cast<std::uint8_t>(color[1] * 255),
                        static_cast<std::uint8_t>(color[2] * 255)});
                }
                ImGui::SameLine();
                ImGui::TextUnformatted(
                    fmt::format("HW: v{}.{}.{} SW: v{}.{}.{}",
                                device->getId().value().hwVersion.major,
                                device->getId().value().hwVersion.minor,
                                device->getId().value().hwVersion.patch,
                                device->getId().value().swVersion.major,
                                device->getId().value().swVersion.minor,
                                device->getId().value().swVersion.patch)
                        .c_str());
            }
            ImGui::EndListBox();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::End();
        ImGui::PopStyleColor(7);
    };

    ImGuiInstance window{1920, 1080, load_json<std::string>(config, "title")};
    while (window.run(render)) {
    }
    return 0;
}
