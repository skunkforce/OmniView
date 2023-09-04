#include "OmniscopeCommunication.hpp"
#include "apihandler.hpp"
#include "jasonhandler.hpp"
#include "settingspopup.hpp"
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
// clang-format off
#include <imfilebrowser.h>
// clang-format on

const std::string configpath = "../config/config.json";

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

void popup_create_training_data_compression(
  nlohmann::json const& config,
  nlohmann::json const& language) {
    static std::string        api_message = " ";
    static ImGui::FileBrowser fileBrowser;
    static ImGui::FileBrowser fileBrowser2;
    static bool               first_job = true;
    static nlohmann::json metadata;

    if(first_job) {
        fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
        fileBrowser2.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
        first_job = false;
    }

    static char inputvin[18]  = "";
    static char mileage[10]   = "";
    static char comment[1000] = "";
    ImGui::SetItemDefaultFocus();
    ImGui::BeginChild("trainingleft", ImVec2(500, 300));
    ImGui::Text("stammdaten");
    ImGui::InputText(
      load_json<std::string>(language, "input", "fin", "label").c_str(),
      inputvin,
      sizeof(inputvin));
    ImGui::InputText(
      load_json<std::string>(language, "input", "mileage", "label").c_str(),
      mileage,
      sizeof(mileage));
    ImGui::InputTextMultiline(
      load_json<std::string>(language, "input", "comment", "label").c_str(),
      comment,
      sizeof(comment));

    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("trainingright", ImVec2(500, 300));

    static float z1, z2, z3, z4;
    static char  path1[255];
    static char  path2[255];
    ImGui::Text("Maximalkommpression in bar");
    ImGui::Text("Zylinder 1: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder1", &z1, 0.0f, 0.0f, "%.2f bar");
    ImGui::Text("Zylinder 2: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder2", &z2, 0.0f, 0.0f, "%.2f bar");
    ImGui::Text("Zylinder 3: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder3", &z3, 0.0f, 0.0f, "%.2f bar");
    ImGui::Text("Zylinder 4: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder4", &z4, 0.0f, 0.0f, "%.2f bar");

    ImGui::Columns(2);
    ImGui::Text("Unterdrückte Zündung");
    ImGui::Text("mit Zündung");
    ImGui::NextColumn();

    ImGui::InputText("##path1", path1, sizeof(path1));
    ImGui::SameLine();
    if(ImGui::Button("File1")) {
        fileBrowser.Open();
    }
    fileBrowser.Display();
    if(fileBrowser.HasSelected()) {
        std::string filepath;
        for(auto const& selectedFile : fileBrowser.GetSelected()) {
            if(!filepath.empty()) {
                filepath += "/";
            }
            filepath += selectedFile.string();
        }

        strcpy(path1, filepath.c_str());

        fileBrowser.ClearSelected();
    }

    ImGui::InputText("##path2", path2, sizeof(path2));
    ImGui::SameLine();
    if(ImGui::Button("File2")) {
        fileBrowser2.Open();
    }
    fileBrowser2.Display();
    if(fileBrowser2.HasSelected()) {
        std::string filepath;
        for(auto const& selectedFile : fileBrowser2.GetSelected()) {
            if(!filepath.empty()) {
                filepath += "/";
            }
            filepath += selectedFile.string();
        }
        strcpy(path2, filepath.c_str());
        fileBrowser2.ClearSelected();
    }
    ImGui::Columns(1);

    if(ImGui::Button(
         load_json<std::string>(language, "button", "send").c_str(),
         ImVec2(load_json<Size>(config, "button"))))
    {
        //Api muss angepasst werden und die funktion send to api ebenso
        metadata["z1"]=z1;
        metadata["z2"]=z2;
        metadata["z3"]=z3;
        metadata["z4"]=z4;
        metadata["kommentar"]=comment;
        metadata["laufleistung"]=mileage;
        metadata["zündung"]="unterdrückt";
        api_message = send_to_api(config, path1, inputvin, "kompressionsmessung",metadata);
        metadata["zündung"]="aktiviert";
        api_message += send_to_api(config, path2, inputvin, "kompressionsmessung",metadata);
    }

    ImGui::SameLine();
    if(ImGui::Button(
         load_json<std::string>(language, "button", "back").c_str(),
         ImVec2(load_json<Size>(config, "button"))))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndChild();
    ImGui::Text(api_message.c_str());
}
void load_settings(nlohmann::json const& config) {
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(load_json<Color>(config, "menubar", "main")));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(load_json<Color>(config, "menubar", "popup")));
    ImGui::PushStyleColor(
      ImGuiCol_Text,
      ImVec4(load_json<Color>(config, "text", "color", "normal")));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(load_json<Color>(config, "window", "color")));

    set_button_style_to(config, "standart");

    ImGuiIO& io        = ImGui::GetIO();
    io.FontGlobalScale = load_json<float>(config, "text", "scale");
}

//###########################################################################
//############# INT MAIN BEGINN #############################################
//###########################################################################


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

    double      xmax_paused{0};
    static bool open_settings     = false;
    bool        captureWindowOpen = true;
    bool        paused            = false;

    static ImVec2 mainMenuBarSize;

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
        load_settings(config);
        ImGui::SetNextWindowPos(ImVec2(0.0f, mainMenuBarSize.y ));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin(
          "OmniScopev2 Data Capture Tool",
          nullptr,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove); //
       
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
                 load_json<std::string>(language, "menubar", "menu", "settings").c_str()))
            {
                open_settings = true;
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
        mainMenuBarSize = ImGui::GetItemRectSize();
        ImGui::EndMainMenuBar();
        
        
        
        

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
        
        ImGui::BeginChild("Buttonstripe", ImVec2(-1, ImVec2(load_json<Size>(config, "button")).y));

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
            ImGui::InputText(
              load_json<std::string>(language, "input", "mileage", "label").c_str(),
              mileage,
              sizeof(mileage));
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
                std::string      filename{fmt::format("{}-{:%Y-%m-%dT%H:%M}.csv", mileage, now)};
                std::filesystem::path path_path = path_sv;
                if(captureData.empty()) {
                    ImGui::CloseCurrentPopup();
                } else {
                    // create the given folder_structure
                    std::filesystem::path first_folder
                      = load_json<std::filesystem::path>(config, "scanfolder");
                    std::filesystem::path complete_path = first_folder / inputvin / scantype;
                    std::filesystem::create_directories(complete_path);

                    save(captureData, path_path / complete_path / filename);

                    savedFileNames.emplace_back(
                      path_path.string(),
                      fmt::format("{:%T}-{:%T}", startTimepoint, now).c_str());
                    // nicht mehr im save-kontext, sondern in create training data
                    //send_to_api(config, path_path / filename, inputvin, scantype);
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
        if(ImGui::BeginPopupModal("createtrainingdata", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetItemDefaultFocus();
            popup_create_training_data_compression(config, language);
            ImGui::EndPopup();
        }
        if(paused == true) {
            set_button_style_to(config, "start");

            if(ImGui::Button("Start", ImVec2(load_json<Size>(config, "button")))) {
                paused = false;
            }
            ImGui::PopStyleColor(3);
            //set_button_style_to(config, "standart");
        } else {
            set_button_style_to(config, "stop");
            if(ImGui::Button("Stop", ImVec2(load_json<Size>(config, "button")))) {
                paused = true;
            }
            ImGui::PopStyleColor(3);
            //set_button_style_to(config, "standart");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save to File", ImVec2(load_json<Size>(config, "button")))) {
            //savecontext = true;//Opens new overlay

            ImGui::OpenPopup("savetofile");
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
        ImGui::FileBrowser fileBrowser;
        if(ImGui::Button("Analyse Data", ImVec2(load_json<Size>(config, "button")))) {
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
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));
        ImGui::SameLine();
        std::string settingstitle = load_json<std::string>(language, "settings", "title");
        if(open_settings == true) {
            ImGui::OpenPopup(settingstitle.c_str());
            open_settings = false;
        }
        if(ImGui::BeginPopupModal(
             settingstitle.c_str(),
             nullptr,
             ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetItemDefaultFocus();
            popup_settings(config, language, configpath);
            ImGui::EndPopup();
        }
        if(ImGui::Button("Create Training Data", ImVec2(load_json<Size>(config, "button")))) {
            ImGui::OpenPopup("createtrainingdata");
        }
        ImGui::PopStyleColor();
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
        //ImGui::EndChild();
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
        ImGui::PopStyleColor(7);
        //ImGui::End();
    };

    ImGuiInstance window{1280, 760, load_json<std::string>(config, "title")};
    while(window.run(render)) {
    }
    return 0;
}
