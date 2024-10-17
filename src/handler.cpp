#include "handler.hpp"
#include "../imgui-stdlib/imgui_stdlib.h"
#include "get_from_github.hpp"
#include "popups.hpp"
#include <functional>
#include <implot.h>
#include <set>

struct AxisInfo {
  const std::pair<Omniscope::Id, std::vector<std::pair<double, double>> &> data;
  std::pair<std::string, ImAxis_> egu;
  std::string timebase;

  AxisInfo(
      const std::pair<Omniscope::Id, std::vector<std::pair<double, double>> &>
          data_,
      std::pair<std::string, ImAxis_> egu_, std::string timebase_)
      : data{data_}, egu{egu_}, timebase{timebase_} {}
};

static std::vector<AxisInfo> plotAxes;

static std::vector<AxisInfo> getDeviceInfos() {
  std::vector<AxisInfo> axisInfos;
  std::vector<Omniscope::Id> samplerDvcs; // store live devices
  std::vector<std::pair<Omniscope::Id, ImAxis_>> assignedEgus;
  if (sampler.has_value())
    for (auto const &device : sampler->sampleDevices) {
      std::string egu =
          device.first->getEgu().value_or(appLanguage[Key::ADC_counts]);
      if (auto id = device.first->getId(); id.has_value()) {
        auto deviceId = id.value();
        samplerDvcs.push_back(deviceId);
        if (captureData.contains(deviceId)) {
          const int maxAxes = 3; // max count of axis

        auto deviceIdExists = std::find_if(assignedEgus.begin(), assignedEgus.end(),
                                                       [&deviceId](const auto& pair) {
                                                           return pair.first == deviceId;
                                                       }) != assignedEgus.end();

              if (!deviceIdExists && assignedEgus.size() < maxAxes) {
                  static_assert((ImAxis_Y1 + 1) == ImAxis_Y2, "Achsenwerte haben sich geändert");

                  ImAxis_ nextYAxis = static_cast<ImAxis_>(ImAxis_Y1 + assignedEgus.size());

                  // New Axis if less than 3 axis are used
                  if (assignedEgus.size() < maxAxes) {
                      assignedEgus.emplace_back(std::make_pair(deviceId, nextYAxis));
                  } else {
                      fmt::print("Zu viele Achsen hinzugefügt. Keine weitere EGU-Achse für: {}\nDevice id: {}", egu, deviceId.serial);
                  }
              } else {
                  // Error if to many axis would be used  
                  fmt::print("Maximale Anzahl an Achsen (3) erreicht oder Achse bereits vorhanden. "
                              "Keine weitere Achse hinzugefügt für: {}\nDevice id: {}", egu, deviceId.serial);
              }

        // Speichern der Achse in axisInfos
        axisInfos.push_back({{deviceId, std::ref(captureData[deviceId])},
                              {egu, assignedEgus.back().second},
                              std::to_string(deviceId.sampleRate)});
      } else
        fmt::println("Error no device id found");
    }
  }
  // also get loaded files info
  for (auto &[device, values] : captureData)
    if (std::ranges::find(samplerDvcs, device.serial, &Omniscope::Id::serial) ==
        samplerDvcs.end())
      axisInfos.push_back({{device, values},
                           {"y [Volts]", ImAxis_Y1},
                           std::to_string(device.sampleRate)});
  return axisInfos;
}

void addPlots(const char *name, fs::path &AnalyzedFilePath, bool &LOADANALYSISDATA, 
              std::function<void(double, std::string, ImAxis_, double, double)>
                  axesSetup) {
  static std::set<std::string> firstRun;
  auto const plotRegion = ImGui::GetContentRegionAvail();
  ImVec2 adjustedPlotRegion(plotRegion.x - 100, plotRegion.y);
  // TODO search devices must work aswell
  plotAxes = getDeviceInfos();

  if (ImPlot::BeginPlot(name, adjustedPlotRegion, ImPlotFlags_NoFrame)) {
  
   if(!AnalyzedFilePath.empty() && LOADANALYSISDATA){
    AddPlotFromFile(AnalyzedFilePath); 
    ImPlot::EndPlot();
    ImPlot::PopStyleColor();
   }
   else {

    // TODO: if bool areFilesLoading = false this , else AddPlotFromFile
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::min();

    for (auto const &axes : plotAxes)
      if (!axes.data.second.empty()) {
        x_max = std::max(x_max, axes.data.second.back().first);
        // TODO save max and min value over same axis
        auto [min, max] = std::minmax_element(axes.data.second.begin(),
                                              axes.data.second.end());
        double yMin = min->first + (min->first * 0.15);
        double yMax = max->second + (max->second * 0.15);
        // fmt::print("yMin {}, yMax{}\n", yMin, yMax);
        axesSetup(x_max, axes.egu.first, axes.egu.second, yMin, yMax);
      }

    auto const limits = [&]() {
      if (!firstRun.contains(name)) {
        firstRun.insert(name);
        return ImPlotRect(x_min, x_max, 0, 0);
      }
      return ImPlot::GetPlotLimits();
    }();

    auto addPlot = [&](auto const &plot, ImAxis_ yAxis) {
      if (!plot.second.empty()) {
        auto const start = [&]() {
          auto p = std::lower_bound(plot.second.begin(), plot.second.end(),
                                    std::pair<double, double>{limits.X.Min, 0});
          return p != plot.second.begin() ? p - 1 : p;
        }();

        auto const end = [&]() {
          auto p = std::upper_bound(start, plot.second.end(),
                                    std::pair<double, double>{limits.X.Max, 0});
          return p != plot.second.end() ? p + 1 : p;
        }();

        std::size_t const stride = [&]() -> std::size_t {
          auto const s = std::distance(start, end) / (plotRegion.x * 2.0);
          return s <= 1 ? 1 : static_cast<std::size_t>(s);
        }();

        // determine which axes is the right one to choose
        ImPlot::SetAxes(ImAxis_X1, yAxis);
        ImPlot::PlotLine(
            fmt::format("{}-{}", plot.first.type, plot.first.serial).c_str(),
            std::addressof(start->first), std::addressof(start->second),
            static_cast<std::size_t>(std::distance(start, end)) / stride, 0, 0,
            2 * sizeof(double) * stride);
      }
    };
    for (auto const &plot : plotAxes) {
      ImPlot::SetNextLineStyle(ImVec4{colorMap[plot.data.first][0],
                                      colorMap[plot.data.first][1],
                                      colorMap[plot.data.first][2], 1.0f});
      addPlot(plot.data, plot.egu.second);
    }
    ImPlot::EndPlot();
   }
  }
}

void parseDeviceMetaData(Omniscope::MetaData metaData,
                         std::shared_ptr<OmniscopeDevice> &device) {
  try {
    nlohmann::json metaJson = nlohmann::json::parse(metaData.data);
    fmt::println("{}", metaJson.dump());
    device->setScale(std::stod(metaJson["scale"].dump()));
    device->setOffset(std::stod(metaJson["offset"].dump()));
    device->setEgu(metaJson["egu"]);
  } catch (...) {
    fmt::print("parsing Meta Data error: {}", metaData.data);
  }
}

void initDevices() {
  constexpr int VID = 0x2e8au;
  constexpr int PID = 0x000au;

  devices = deviceManager.getDevices(VID, PID);
  for (auto &device : devices) {
    auto metaDataCb = [&](auto const &msg) {
      if (std::holds_alternative<Omniscope::MetaData>(msg)) {
        parseDeviceMetaData(std::get<Omniscope::MetaData>(msg), device);
      }
    };
    auto id = device->getId().value();
    auto sampleRate = static_cast<double>(id.sampleRate);
    device->setTimeScale(static_cast<double>(1 / sampleRate));
    if (!colorMap.contains(id)) {
      ImPlot::PushColormap(ImPlotColormap_Dark);
      auto c = ImPlot::GetColormapColor((colorMap.size() % 7) + 1);
      colorMap[id] = std::array<float, 3>{c.x, c.y, c.z};
      ImPlot::PopColormap();
    }
    auto &color = colorMap[id];
    device->send(Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                                   static_cast<std::uint8_t>(color[1] * 255),
                                   static_cast<std::uint8_t>(color[2] * 255)});
    // set Callback for MetaData
    device->setMessageCallback(metaDataCb);
    device->send(Omniscope::GetMetaData{});
  }
}

void devicesList(bool const &flagPaused) {
  auto doDevice = [&](auto &device, auto msg) {
    auto &color = colorMap[device->getId().value()];
    if (ImGui::ColorEdit3(
            fmt::format("{:<32}",
                        fmt::format("{}-{}", device->getId().value().type,
                                    device->getId().value().serial))
                .c_str(),
            color.data(),
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker |
                ImGuiColorEditFlags_NoTooltip)) {
      device->send(
          Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                            static_cast<std::uint8_t>(color[1] * 255),
                            static_cast<std::uint8_t>(color[2] * 255)});
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(fmt::format("HW: v{}.{}.{} SW: v{}.{}.{}    ",
                                       device->getId().value().hwVersion.major,
                                       device->getId().value().hwVersion.minor,
                                       device->getId().value().hwVersion.patch,
                                       device->getId().value().swVersion.major,
                                       device->getId().value().swVersion.minor,
                                       device->getId().value().swVersion.patch)
                               .c_str());
    ImGui::SameLine();
    if (device->isRunning())
      ImGui::TextUnformatted(fmt::format("{}", msg).c_str());
    else
      ImGui::TextUnformatted("Error");
  };

  if (sampler.has_value())
    for (auto &device : sampler->sampleDevices) {
      if (!flagPaused) {
        doDevice(device.first, appLanguage[Key::Measurement]);
      } else {
        doDevice(device.first, appLanguage[Key::Stop]);
      }
    }
  else
    for (auto &device : devices)
      doDevice(device, appLanguage[Key::Ready]);
}

void load_files(decltype(captureData) &loadedFiles,
                std::map<Omniscope::Id, std::string> &loadedFilenames,
                bool &loadFile) {
  static std::set<fs::path> loadedFilePaths;
  auto do_load = [&loadedFiles, &loadedFilenames] {
    std::pair<Omniscope::Id, std::vector<std::pair<double, double>>> loadedFile;
    for (const auto &path : loadedFilePaths) {
      std::ifstream readfile(path, std::ios::binary);
      if (!readfile.is_open())
        fmt::println("Failed to open file {}", path.string());
      else {
        std::string first_line;
        std::getline(readfile, first_line);
        std::istringstream input{first_line};
        static constexpr size_t fieldsSz{6};
        // extract input fields data from the first line
        for (size_t i = 0; i < fieldsSz; i++) {
          std::string substr;
          std::getline(input, substr, ',');
          if (i == 3) // fourth element (Type of scope)
            loadedFile.first.type = substr;
          if (i == 4) // fifth element (serial of scope)
            loadedFile.first.serial = substr;
        }
        // each y-value is recorded at 1/sampleRate time
        double step{0.00001}, base{step};
        size_t indx{2};           // y_values start from line 2 of the file
        while (!readfile.eof()) { // fill the vector of the values
          double value{};
          readfile >> value;
          loadedFile.second.emplace_back(base, value);
          static constexpr size_t bigNumber{10'000'000};
          readfile.ignore(bigNumber,
                          '\n'); // new line separator between elements
          base += step;
        }
        readfile.close();
        loadedFile.second.pop_back(); // pop the extra last element
        loadedFiles.emplace(loadedFile);
        loadedFilenames.emplace(loadedFile.first, path.filename().string());
      }
    }
    loadedFilePaths.clear();
  };

  if (ImGui::BeginPopupModal(appLanguage[Key::Load_file_data], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    static ImGui::FileBrowser fileBrowser;
    static std::vector<std::string> pathArr;
    if (pathArr.empty())
      pathArr.emplace_back("");

    for (auto &path : pathArr) {
      ImGui::PushID(&path); // unique IDs
      ImGui::InputTextWithHint("##", appLanguage[Key::Path], &path);
      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Browse]))
        fileBrowser.Open();
      fileBrowser.Display();
      info_popup(appLanguage[Key::Wrong_file_warning],
                 appLanguage[Key::Wrong_file_type]);
      if (fileBrowser.HasSelected()) {
        path = fileBrowser.GetSelected().string();
        fileBrowser.ClearSelected();
      }
      if (!path.empty())
        if (fs::path(path).extension() != ".csv") {
          ImGui::OpenPopup(appLanguage[Key::Wrong_file_warning],
                           ImGuiPopupFlags_NoOpenOverExistingPopup);
          path.clear();
        } else
          loadedFilePaths.emplace(path);
      ImGui::PopID();
    }
    if (ImGui::Button(" + "))
      pathArr.emplace_back("");
    ImGui::SetItemTooltip(appLanguage[Key::Load_another_file]);
    if (ImGui::Button(appLanguage[Key::Back])) {
      pathArr.clear();
      loadedFilePaths.clear();
      loadFile = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(appLanguage[Key::Load_file])) {
      pathArr.clear();
      do_load();
      loadFile = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void set_config(const std::string &configpath) {
  if (fs::exists(configpath))
    fmt::print("found config.json\n\r");
  else {
    fmt::print("Did not find config.json.\n Download from Github\n\r");
    update_config_from_github();
  }
}

void set_json(nlohmann::json &config) {
  if (fs::exists(load_json<std::string>(config, ("languagepath"))))
    fmt::print("Found language: {}\n\r", appLanguage[Key::German]);
  else {
    fmt::print("Did not find {}.\n Download from Github\n\r",
               appLanguage[Key::German]);
    update_language_from_github();
  }
}

void set_inital_config(nlohmann::json &config) {
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = config["text"]["scale"];
  appLanguage =
      config["text"]["active_language"] == "German" ? germanLan : englishLan;
}

void rstSettings(const decltype(captureData) &loadedFiles) {
  sampler.reset();
  devices.clear();
  savedFileNames.clear();
  deviceManager.clearDevices();
  plotAxes.clear();
  // erase all elements excpet loadedFiles
  for (auto it = captureData.begin(); it != captureData.end();) {
    if (!loadedFiles.contains(it->first))
      it = captureData.erase(it);
    else
      ++it;
  }
}

//TODO : Set this also up for saved OmniScope files 

void AddPlotFromFile(fs::path &filePath) {
    LoadedFiles loadedFile;  
    loadedFile.LoadFromFile(filePath); 
    
       if (loadedFile.units.size() >= 2) {
        ImPlot::SetupAxis(ImAxis_X1, loadedFile.units[0].c_str());
        ImPlot::SetupAxis(ImAxis_Y1, loadedFile.units[1].c_str());
    } else {
        // If the user used a wrong file or the analysis went wrong 
        std::cerr << "Error: Not enough units provided for axis labels." << std::endl;
        return;
    }

    std::map<double, double> aggregated_data;

  
    std::vector<double> x_values;
    std::vector<double> y_values;
    
    std::vector<double> filtered_x_values;
    std::vector<double> filtered_y_values;

    for (const auto& pair : loadedFile.data) {
        x_values.push_back(pair.first);
        y_values.push_back(pair.second);
    }

    for (size_t i = 0; i < x_values.size(); ++i) {
        if (x_values[i] >= 1 && x_values[i] <= 12500) { // only fre between 1 and 12500 hz as well as rounded freq 
            double rounded_x = std::round(x_values[i]);

            if (aggregated_data.find(rounded_x) != aggregated_data.end()) {
                aggregated_data[rounded_x] += y_values[i]; 
            } else {
                aggregated_data[rounded_x] = y_values[i];
            }
        }
    }

    for (const auto& pair : aggregated_data) {
        filtered_x_values.push_back(pair.first);  
        filtered_y_values.push_back(pair.second); 
    }

    if (!filtered_x_values.empty() && !filtered_y_values.empty()) {
      ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4{0.686f, 0.0f, 0.007f, 1.000f});
      ImPlot::SetNextLineStyle(ImVec4{0.686f, 0.0f, 0.007f, 1.000f}); 

        ImPlot::PlotBars(filePath.string().c_str(),
                        filtered_x_values.data(),
                        filtered_y_values.data(),
                        static_cast<int>(filtered_x_values.size()),
                        0.001,0,0,
                        sizeof(double));

    } 
}


void LoadedFiles::LoadFromFile(fs::path &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Fehler: Datei konnte nicht geöffnet werden." << std::endl;
    }
    else {

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;

        // Erste Zeile: Metadaten
        if (lineNumber == 1) {
      
        }
        // Zweite Zeile: Einheiten
        else if (lineNumber == 2) {
            parseUnits(line);
        }
        // Ab der dritten Zeile: Datenpaare
        else {
            parseData(line);
        }
    }

    file.close();
    }
}

void LoadedFiles::printData(){

    std::cout << "Einheiten: ";
    for (const auto& unit : units) {
        std::cout << unit << " ";
    }
    std::cout << std::endl;

    std::cout << "Daten: " << std::endl;
    for (const auto& pair : data) {
        std::cout << pair.first << ", " << pair.second << std::endl;
    }
}

// Private Methoden zur Verarbeitung der CSV-Daten
void LoadedFiles::parseUnits(const std::string& line) {
    std::stringstream ss(line);
    std::string unit;

    // Einheiten durch Kommas getrennt
    while (std::getline(ss, unit, ',')) {
        units.push_back(trim(unit)); // to not load space or \n 
    }
}

std::string trim(const std::string& str) {
    std::string trimmed = str;
    trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());
    return trimmed;
}

void LoadedFiles::parseData(const std::string& line) {
    std::stringstream ss(line);
    std::string value1_str, value2_str;

    // Lese die beiden Werte als Strings, getrennt durch Komma oder Leerzeichen
    std::getline(ss, value1_str, ',');  // Lese den ersten Wert bis zum Komma
    std::getline(ss, value2_str);       // Lese den zweiten Wert (der Rest der Zeile)

    // Entferne eventuelle zusätzliche Leerzeichen
    value1_str.erase(remove_if(value1_str.begin(), value1_str.end(), isspace), value1_str.end());
    value2_str.erase(remove_if(value2_str.begin(), value2_str.end(), isspace), value2_str.end());

    // Konvertiere die Strings in double-Werte
    double value1 = std::stod(value1_str);
    double value2 = std::stod(value2_str);

    // Speichere das Paar in der Datenstruktur
    data.emplace_back(value1, value2);
}