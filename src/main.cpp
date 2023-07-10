#include "OmniscopeCommunication.hpp"

#include <ImGuiInstance/ImGuiInstance.hpp>
#include <algorithm>
#include <boost/asio.hpp>
#include <curl/curl.h>
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

const std::string configpath = "../config/config.json";

struct Color : ImVec4 {
    using ImVec4::ImVec4;
};
struct Size : ImVec2 {
    using ImVec2::ImVec2;
};

void to_json(nlohmann::json& j, Color const& c) {
    j = nlohmann::json{
      {         "red", c.x},
      {       "green", c.y},
      {        "blue", c.z},
      {"transparency", c.w}
    };
}

void from_json(nlohmann::json const& j, Color& c) {
    j.at("red").get_to(c.x);
    j.at("green").get_to(c.y);
    j.at("blue").get_to(c.z);
    j.at("transparency").get_to(c.w);
}
void to_json(nlohmann::json& j, Size const& s) {
    j = nlohmann::json{
      {"sizex", s.x},
      {"sizey", s.y},
    };
}

void from_json(nlohmann::json const& j, Size& s) {
    j.at("sizex").get_to(s.x);
    j.at("sizey").get_to(s.y);
}

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
      ImVec4(load_json<Color>(config, "button", name, "normal")));
    ImGui::PushStyleColor(
      ImGuiCol_ButtonHovered,
      ImVec4(load_json<Color>(config, "button", name, "hover")));
    ImGui::PushStyleColor(
      ImGuiCol_ButtonActive,
      ImVec4(load_json<Color>(config, "button", name, "active")));
}

//template<typename T>
bool send_to_api(
  nlohmann::json const& config,
  std::string const&    file,
  std::string const&    vin,
  std::string const&    scantype) {
    fmt::print("send to api\n\r");
    CURL*    curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string url = load_json<std::string>(config, "api", "url") + "?"
                        + load_json<std::string>(config, "api", "input1") + "=" + vin + "&"
                        + load_json<std::string>(config, "api", "input2") + "=" + scantype;

        fmt::print("url: {}\n\r", url);

        // Hinzufügen der Daten als Formfelder
        curl_mime*     mime = curl_mime_init(curl);
        curl_mimepart* part;
        fmt::print("mime init\n\r");

        // Datei

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, file.c_str());

        struct curl_slist* header_list = NULL;
        header_list                    = curl_slist_append(
          header_list,
          load_json<std::string>(config, "api", "header1").c_str());
        header_list = curl_slist_append(
          header_list,
          load_json<std::string>(config, "api", "header2").c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        // Senden der Anfrage
        fmt::print("senden der Anfrage\n\r");
        res = curl_easy_perform(curl);

        // Überprüfung auf Fehler
        fmt::print("prüfe auf fehler\n\r");
        if(res != CURLE_OK)
            fmt::print("Fehler beim Hochladen der Datei:{}\n\r ", curl_easy_strerror(res));

        // Aufräumen
        curl_easy_cleanup(curl);
        curl_slist_free_all(header_list);
    } else {
        fmt::print("senden erfoldgreich\n\r");
    }

    // Aufräumen von cURL

    curl_global_cleanup();
    fmt::print("schließe send to api\n\r");
    return false;
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
          ImVec4(load_json<Color>(config, "menubar", "main")));
        ImGui::PushStyleColor(
          ImGuiCol_PopupBg,
          ImVec4(load_json<Color>(config, "menubar", "popup")));
        ImGui::PushStyleColor(
          ImGuiCol_Text,
          ImVec4(load_json<Color>(config, "text", "color", "normal")));
        ImGui::PushStyleColor(
          ImGuiCol_WindowBg,
          ImVec4(load_json<Color>(config, "window", "color")));
        set_button_style_to(config, "standart");

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
        if(ImGui::BeginPopupModal("savetofile", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetItemDefaultFocus();

            static char inputvin[18]  = "";
            static char mileage[10]   = "";
            static char scantype[255] = "";
            ImGui::SetItemDefaultFocus();
            ImGui::InputText(
              load_json<std::string>(language, "input", "fin", "label").c_str(),
              inputvin,
              sizeof(inputvin));
            /*ImGui::InputText(
              load_json<std::string>(language, "input", "mileage", "label").c_str(),
              mileage,
              sizeof(mileage));*/
            ImGui::InputText(
              load_json<std::string>(language, "input", "scantype", "label").c_str(),
              scantype,
              sizeof(scantype));

            if(ImGui::Button(
                 load_json<std::string>(language, "button", "save").c_str(),
                 ImVec2(load_json<Size>(config, "button"))))
            {
                now = std::chrono::system_clock::now();

                now_time_t = std::chrono::system_clock::to_time_t(now);
                now_tm     = *std::gmtime(&now_time_t);

                std::string_view path_sv{path.data()};

                std::string filename{fmt::format("{:%Y-%m-%dT%H:%M:%S_%z_%Z}.csv", now)};

                std::filesystem::path path_path = path_sv;
                if(captureData.empty()) {
                    ImGui::CloseCurrentPopup();
                } else {
                    save(captureData, path_path / filename);

                    savedFileNames.emplace_back(
                      path_path.string(),
                      fmt::format("{:%T}-{:%T}", startTimepoint, now).c_str());

                    send_to_api(config, path_path / filename, inputvin, scantype);
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if(ImGui::Button(
                 load_json<std::string>(language, "button", "back").c_str(),
                 ImVec2(load_json<Size>(config, "button"))))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if(paused == true) {
            set_button_style_to(config, "start");

            if(ImGui::Button("Start", ImVec2(load_json<Size>(config, "button")))) {
                paused = false;
            }
            set_button_style_to(config, "standart");
        } else {
            set_button_style_to(config, "stop");
            if(ImGui::Button("Stop", ImVec2(load_json<Size>(config, "button")))) {
                paused = true;
            }
            set_button_style_to(config, "standart");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save to File", ImVec2(load_json<Size>(config, "button")))) {
            //savecontext = true;//Opens new overlay
            ImGui::OpenPopup("savetofile");
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

        ImGui::Button("Analyse Data", ImVec2(load_json<Size>(config, "button")));
        ImGui::PushStyleColor(ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));
        ImGui::SameLine();
        ImGui::FileBrowser fileBrowser;
        if(ImGui::Button("Create Training Data", ImVec2(load_json<Size>(config, "button")))) {
            fileBrowser.Open();
        }

        fileBrowser.Display();
        if(fileBrowser.HasSelected()) {
            // Hier kannst du auf die ausgewählten Dateien zugreifen
            for(const auto& selectedFile : fileBrowser.GetSelected()) {
                std::string filename = selectedFile.string();
                // Verarbeite die ausgewählte Datei
                // ...
            }

            fileBrowser.ClearSelected();
        }
        ImGui::SameLine();
        if(ImGui::Button("Refresh Devicelist", ImVec2(load_json<Size>(config, "button")))) {
            newDevices = Omniscope::queryDevices();
        }

        ImGui::EndChild();

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
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Live Capture", ImVec2(-1, -1));

        center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        //ImGui::CloseCurrentPopup();)
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

        ImGui::End();
    };

    ImGuiInstance window{1280, 760, load_json<std::string>(config, "title")};
    while(window.run(render)) {
    }
    return 0;
}
