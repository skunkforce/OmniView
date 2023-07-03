#include "OmniscopeCommunication.hpp"

#include <ImGuiInstance/ImGuiInstance.hpp>
#include <algorithm>
#include <boost/asio.hpp>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <thread>

const std::string configpath = "../config/config.json";

nlohmann::json load_json_file(std::string const& path) {
    std::ifstream  json_file(path);
    nlohmann::json return_json;
    json_file >> return_json;
    return return_json;
}

void write_json_file(std::string const& path, nlohmann::json const& json_data) {
    std::ofstream json_file(path);
    json_file << json_data.dump(4);
    json_file.close();
}

template<typename T>
T load_json(nlohmann::json const& configjson, std::string const& key) {
    if(configjson.contains(key)) {
        return configjson[key].get<T>();
    } else {
        fmt::print("key {} not found\n\r", key);
        if constexpr(std::is_same_v<T, std::string>) {
            return T{key};
        } else {
            return T{};
        }
    }
}

template<typename T, typename... Args>
T load_json(nlohmann::json const& config, std::string const& key, Args... keylist) {
    nlohmann::json unpackme = config[key];
    T              returnme = load_json<T>(unpackme, keylist...);

    return returnme;
}

std::vector<std::string> getAvailableLanguages(std::string const& languageFolder) {
    std::vector<std::string> languages;
    for(auto const& entry : std::filesystem::directory_iterator(languageFolder)) {
        if(entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if(extension == ".json") {
                std::string filename = entry.path().filename().stem().string();
                languages.push_back(filename);
            }
        }
    }
    return languages;
}

static void save(
  std::map<Omniscope::Device, std::vector<std::pair<double, double>>> const& alignedData,
  std::filesystem::path const&                                               outFile) {
    auto minSize = std::numeric_limits<std::size_t>::max();

    std::vector<std::vector<std::pair<double, double>> const*> data;

    std::string fileContent;

    for(auto sep = std::string_view{}; auto const& [device, values] : alignedData) {
        fileContent += sep;
        fileContent += "\"";
        fileContent += fmt::format("{}-{}", device.type, device.serial);
        fileContent += "\"";
        sep     = ",";
        minSize = std::min(values.size(), minSize);
        data.push_back(std::addressof(values));
    }
    fileContent += '\n';

    for(std::size_t i{}; i < minSize; ++i) {
        fileContent += fmt::format("{}", i);
        fileContent += ",";
        for(auto sep = std::string_view{}; auto const& vec : data) {
            auto const dataPair = (*vec)[i];
            fileContent += sep;
            if(dataPair.second) {
                fileContent += fmt::format("{}", dataPair.second);
            }
            sep = ",";
        }
        fileContent += '\n';
    }

    fileContent += '\n';

    auto path = outFile;
    path.remove_filename();
    if(!std::filesystem::exists(path) && !path.empty()) {
        std::filesystem::create_directories(path);
    }

    fmt::print("start save {}\n", outFile.string());
    std::ofstream file{outFile};
    file << fileContent;
    file.close();

    fmt::print("finished save\n");
}

void set_button_style_to(nlohmann::json const& config, std::string const& name) {
    ImGui::PushStyleColor(
      ImGuiCol_Button,
      ImVec4(
        load_json<float>(config, "button", name, "normal", "red"),
        load_json<float>(config, "button", name, "normal", "green"),
        load_json<float>(config, "button", name, "normal", "blue"),
        load_json<float>(config, "button", name, "normal", "transparency")));
    ImGui::PushStyleColor(
      ImGuiCol_ButtonHovered,
      ImVec4(
        load_json<float>(config, "button", name, "hover", "red"),
        load_json<float>(config, "button", name, "hover", "green"),
        load_json<float>(config, "button", name, "hover", "blue"),
        load_json<float>(config, "button", name, "hover", "transparency")));
    ImGui::PushStyleColor(
      ImGuiCol_ButtonActive,
      ImVec4(
        load_json<float>(config, "button", name, "active", "red"),
        load_json<float>(config, "button", name, "active", "green"),
        load_json<float>(config, "button", name, "active", "blue"),
        load_json<float>(config, "button", name, "active", "transparency")));
}

int main() {
    nlohmann::json config = load_json_file(configpath);

    std::vector<std::string> availableLanguages
      = getAvailableLanguages(load_json<std::string>(config, "languagepath"));

    nlohmann::json language = load_json_file(
      load_json<std::string>(config, "languagepath") + load_json<std::string>(config, "language")
      + ".json");

    static constexpr std::size_t   captureDataReserve = 1 << 26;
    std::vector<Omniscope::Device> devices;
    std::vector<Omniscope::Device> newDevices;

    std::vector<std::pair<std::string, std::string>> savedFileNames{};
    std::optional<Omniscope::DeviceRunner>           runner;

    auto        startTimepoint = std::chrono::system_clock::now();
    auto        now            = std::chrono::system_clock::now();
    std::time_t now_time_t     = std::chrono::system_clock::to_time_t(now);
    std::tm     now_tm         = *std::gmtime(&now_time_t);

    double xmax_paused{0};

    bool captureWindowOpen = true;
    bool paused            = false;

    std::map<Omniscope::Device, std::vector<std::pair<double, double>>> captureData;

    std::string path;
    path.resize(256);

    auto initRunner = [&]() {
        if(!devices.empty()) {
            try {
                runner.emplace(devices);
                captureData.clear();
                for(auto const& dev : devices) {
                    captureData[dev].reserve(captureDataReserve);
                }
            } catch(std::exception const& e) {
                runner.reset();
                fmt::print(stderr, "Error starting capture: {}\n", e.what());
            }
        }
    };

    auto addPlots
      = [firstRun
         = std::set<std::string>{}](auto const& name, auto const& plots, auto axesSetup) mutable {
            auto const plotRegion = ImGui::GetContentRegionAvail();
            if(ImPlot::BeginPlot(name, plotRegion)) {
                double x_min = std::numeric_limits<double>::max();
                double x_max = std::numeric_limits<double>::min();

                for(auto const& plot : plots) {
                    if(!plot.second.empty()) {
                        x_min = std::min(x_min, plot.second.front().first);
                        x_max = std::max(x_max, plot.second.back().first);
                    }
                }

                axesSetup(x_min, x_max);

                auto const limits = [&]() {
                    if(!firstRun.contains(name)) {
                        firstRun.insert(name);
                        return ImPlotRect(x_min, x_max, 0, 0);
                    }
                    return ImPlot::GetPlotLimits();
                }();

                auto addPlot = [&](auto const& plot) {
                    if(!plot.second.empty()) {
                        auto const start = [&]() {
                            auto p = std::lower_bound(
                              plot.second.begin(),
                              plot.second.end(),
                              std::pair<double, double>{limits.X.Min, 0});
                            if(p != plot.second.begin()) {
                                return p - 1;
                            }
                            return p;
                        }();

                        auto const end = [&]() {
                            auto p = std::upper_bound(
                              start,
                              plot.second.end(),
                              std::pair<double, double>{limits.X.Max, 0});
                            if(p != plot.second.end()) {
                                return p + 1;
                            }
                            return p;
                        }();

                        std::size_t const stride = [&]() -> std::size_t {
                            auto const s = std::distance(start, end) / (plotRegion.x * 2.0);
                            if(1 >= s) {
                                return 1;
                            }
                            return static_cast<std::size_t>(s);
                        }();

                        ImPlot::PlotLine(
                          fmt::format("{}-{}", plot.first.type, plot.first.serial).c_str(),
                          std::addressof(start->first),
                          std::addressof(start->second),
                          static_cast<std::size_t>(std::distance(start, end)) / stride,
                          0,
                          0,
                          2 * sizeof(double) * stride);
                    }
                };

                for(auto const& plot : plots) {
                    addPlot(plot);
                    ImPlot::NextColormapColor();
                }

                ImPlot::EndPlot();
            }
        };

    auto render = [&]() {
        ImGui::SetWindowFontScale(load_json<float>(config, "text", "scale"));
        ImGui::PushStyleColor(
          ImGuiCol_MenuBarBg,
          ImVec4(
            load_json<float>(config, "menubar", "main", "red"),
            load_json<float>(config, "menubar", "main", "green"),
            load_json<float>(config, "menubar", "main", "blue"),
            load_json<float>(config, "menubar", "main", "transparency")));
        ImGui::PushStyleColor(
          ImGuiCol_PopupBg,
          ImVec4(
            load_json<float>(config, "menubar", "popup", "red"),
            load_json<float>(config, "menubar", "popup", "green"),
            load_json<float>(config, "menubar", "popup", "blue"),
            load_json<float>(config, "menubar", "popup", "transparency")));
        ImGui::PushStyleColor(
          ImGuiCol_Text,
          ImVec4(
            load_json<float>(config, "text", "color", "normal", "red"),
            load_json<float>(config, "text", "color", "normal", "green"),
            load_json<float>(config, "text", "color", "normal", "blue"),
            load_json<float>(config, "text", "color", "normal", "transparency")));
        ImGui::PushStyleColor(
          ImGuiCol_WindowBg,
          ImVec4(
            load_json<float>(config, "window", "color", "red"),
            load_json<float>(config, "window", "color", "green"),
            load_json<float>(config, "window", "color", "blue"),
            load_json<float>(config, "window", "color", "transparency")));
        set_button_style_to(config, "stop");

        ImGui::BeginMainMenuBar();
        if(ImGui::BeginMenu(load_json<std::string>(language, "menubar", "menu", "label").c_str())) {
            if(ImGui::BeginMenu(
                 load_json<std::string>(language, "menubar", "menu", "language_option").c_str()))
            {
                for(const auto& lang : availableLanguages) {
                    if(ImGui::MenuItem(lang.c_str())) {
                        config["language"] = lang;
                        write_json_file(configpath, config);
                    }
                }

                ImGui::EndMenu();
            }

            if(ImGui::MenuItem(
                 load_json<std::string>(language, "menubar", "menu", "cleardata").c_str()))
            {
                savedFileNames.clear();
            }
            if(ImGui::MenuItem(
                 load_json<std::string>(language, "menubar", "menu", "clearcapture").c_str()))
            {
                captureData.clear();
            }
            if(ImGui::MenuItem(
                 load_json<std::string>(language, "menubar", "menu", "reset").c_str()))
            {
                runner.reset();
                devices.clear();
                captureData.clear();
                captureWindowOpen = true;
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu(load_json<std::string>(language, "menubar", "view", "label").c_str())) {
            ImGui::MenuItem(load_json<std::string>(language, "menubar", "view", "first").c_str());
            ImGui::MenuItem(load_json<std::string>(language, "menubar", "view", "second").c_str());
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu(load_json<std::string>(language, "menubar", "help", "label").c_str())) {
            ImGui::MenuItem(load_json<std::string>(language, "menubar", "help", "first").c_str());
            ImGui::MenuItem(load_json<std::string>(language, "menubar", "help", "second").c_str());
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin(
          "OmniScopev2 Data Capture Tool",
          nullptr,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove);

        //main menu
        ImGui::BeginChild("Live Capture", ImVec2(-1, 400));
        if(runner && captureWindowOpen == true) {
            if(!paused) {
                if(!runner->copyOut(captureData)) {
                    runner.reset();
                    devices.clear();
                    captureWindowOpen = true;
                    ImGui::OpenPopup("Error!");
                }
            }

            addPlots("Capture", captureData, [&paused, &xmax_paused](auto x_min, auto x_max) {
                if(!paused) {
                    ImPlot::SetupAxes(
                      "x [data points]",
                      "y [ADC value]",
                      ImPlotAxisFlags_AutoFit,
                      ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisLimits(
                      ImAxis_X1,
                      x_max - 2000000,
                      x_max + 1500,
                      ImGuiCond_Always);
                } else {
                    xmax_paused = x_max;
                    ImPlot::SetupAxes("x [data points]", "y [ADC value]", 0, 0);
                }
            });

        } else {
            runner.reset();
            devices.clear();
            captureWindowOpen = true;
        }

        ImGui::EndChild();
        ImGui::BeginChild("Buttonstripe", ImVec2(-1, 40));
        if(paused == true) {
            set_button_style_to(config, "start");
            if(ImGui::Button(
                 "Start",
                 ImVec2(
                   load_json<int>(config, "button", "sizex"),
                   load_json<int>(config, "button", "sizey"))))
            {
                paused = false;
            }
            set_button_style_to(config, "standart");
        } else {
            set_button_style_to(config, "stop");
            if(ImGui::Button(
                 "Stop",
                 ImVec2(
                   load_json<int>(config, "button", "sizex"),
                   load_json<int>(config, "button", "sizey"))))
            {
                paused = true;
            }
            set_button_style_to(config, "standart");
        }
        ImGui::SameLine();
        if(ImGui::Button(
             "Save to File",
             ImVec2(
               load_json<int>(config, "button", "sizex"),
               load_json<int>(config, "button", "sizey"))))
        {
            //Datatype: std::map<Omniscope::Device, std::vector<std::pair<double, double>>> captureData;
            //std::vector<std::pair<double,double>> vecTemp{};
            now = std::chrono::system_clock::now();

            now_time_t = std::chrono::system_clock::to_time_t(now);
            now_tm     = *std::gmtime(&now_time_t);

            std::string_view path_sv{path.data()};

            std::string filename{fmt::format("{:%Y-%m-%dT%H:%M:%S_%z_%Z}.csv", now)};

            std::filesystem::path path_path = path_sv;
            //TODO Implement Feature later. Save the whole capture or the range which is displayed while paused
            //if(!paused){
            //save whole vector
            save(captureData, path_path / filename);
            savedFileNames.emplace_back(
              path_path.string(),
              fmt::format("{:%T}-{:%T}", startTimepoint, now).c_str());
            //}else{
            //save only to xmax saved while paused
            //std::ranges::copy_n(vector.begin(), xmax_paused, std::back_inserter(vecTemp));
            //save(vecTemp, filename));
            //}
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(
          ImGuiCol_Text,
          ImVec4(
            load_json<float>(config, "text", "color", "inactive", "red"),
            load_json<float>(config, "text", "color", "inactive", "green"),
            load_json<float>(config, "text", "color", "inactive", "blue"),
            load_json<float>(config, "text", "color", "inactive", "transparency")));
        ImGui::Button(
          "Analyse Data",
          ImVec2(
            load_json<int>(config, "button", "sizex"),
            load_json<int>(config, "button", "sizey")));
        ImGui::PushStyleColor(
          ImGuiCol_Text,
          ImVec4(
            load_json<float>(config, "text", "color", "normal", "red"),
            load_json<float>(config, "text", "color", "normal", "green"),
            load_json<float>(config, "text", "color", "normal", "blue"),
            load_json<float>(config, "text", "color", "normal", "transparency")));
        ImGui::SameLine();
        ImGui::Button(
          "Create Training Data",
          ImVec2(
            load_json<int>(config, "button", "sizex"),
            load_json<int>(config, "button", "sizey")));
        ImGui::SameLine();
        if(ImGui::Button(
             "Refresh Devicelist",
             ImVec2(
               load_json<int>(config, "button", "sizex"),
               load_json<int>(config, "button", "sizey"))))
        {
            newDevices = Omniscope::queryDevices();
        }
        ImGui::EndChild();
        //ImGui::BeginChild("test", ImVec2(300, -1));
        //this has to die
        ImGui::BeginChild("Devicelist", ImVec2(-1, 300));

        for(auto const& device : newDevices) {
            if(ImGui::Button(fmt::format("{}-{}", device.type, device.serial).c_str())) {
                devices.push_back(device);
                initRunner();
            };
        }
        ImGui::EndChild();

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if(ImGui::BeginPopupModal("Restart?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Restart with the this devices?");
            if(ImGui::BeginListBox("new Devices")) {
                for(auto const& device : newDevices) {
                    ImGui::TextUnformatted(
                      fmt::format("{}-{}", device.type, device.serial).c_str());
                }
                ImGui::EndListBox();
            }

            if(ImGui::Button("OK", ImVec2(120, 0))) {
                devices = newDevices;
                newDevices.clear();
                initRunner();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if(ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if(ImGui::BeginListBox("Current Devices")) {
            for(auto const& device : devices) {
                ImGui::TextUnformatted(fmt::format("{}-{}", device.type, device.serial).c_str());
            }
            ImGui::EndListBox();
        }

        ImGui::InputText("Path", path.data(), path.size());

        if(ImGui::Button("Search new Devices")) {
            newDevices = Omniscope::queryDevices();
            //     if(devices != newDevices) {
            ImGui::OpenPopup("Restart?");
            //   }
        }
        /*######################################################################
        if(ImGui::BeginListBox("Saved files")) {
            for(auto const& filename : savedFileNames) {
                ImGui::TextUnformatted(
                  fmt::format("{} {}", filename.first, filename.second).c_str());
            }
            ImGui::EndListBox();
        }*/

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Live Capture", ImVec2(-1, -1));

        center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if(ImGui::BeginPopupModal("Error!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Error with device!");
            ImGui::Text("Capture data is still valid you can save it!");
            ImGui::SetItemDefaultFocus();
            if(ImGui::Button("Ok", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::EndChild();
        ImGui::End();
    };

    ImGuiInstance window{1280, 760, load_json<std::string>(config, "title")};
    while(window.run(render)) {
    }
    return 0;
}
