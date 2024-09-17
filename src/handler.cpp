#include "handler.hpp"
#include <functional>
#include <set>
#include <implot.h>
#include "popups.hpp"
#include "get_from_github.hpp"
#include "../imgui-stdlib/imgui_stdlib.h"

struct AxisInfo {
  const std::pair<Omniscope::Id, std::vector<std::pair<double, double>> &> data;
  std::pair<std::string, ImAxis_> egu;
  std::string timebase; // store sampleRate

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
  std::vector<std::pair<std::string, ImAxis_>> assignedEgus;
  if (sampler.has_value())
    for (auto const &device : sampler->sampleDevices) {
      std::string egu =
          device.first->getEgu().value_or(appLanguage[Key::ADC_counts]);
      if (auto id = device.first->getId(); id.has_value()) {
        auto deviceId = id.value();
        samplerDvcs.push_back(deviceId);
        if (captureData.contains(deviceId)) {
          if (!std::ranges::contains(assignedEgus, egu,
                                     &std::pair<std::string, ImAxis_>::first))
            if (assignedEgus.size() <= 3) {
              // make sure ImAxis_ values haven't changed
              static_assert((ImAxis_Y1 + 1) == ImAxis_Y2);
              ImAxis_ nextYAxis =
                  static_cast<ImAxis_>(ImAxis_Y1 + assignedEgus.size());
              assignedEgus.push_back({egu, nextYAxis});
            } else {
              fmt::println("too many Axes added, egu not added: "
                           "{}\nDevice id: {}",
                           egu, id.value());
              break;
            }
          axisInfos.push_back({{deviceId, captureData[deviceId]},
                               assignedEgus.back(),
                               std::to_string(deviceId.sampleRate)});
        }
      } else
        fmt::println("Error no device id found");
    }
  // also get loaded files info
  for (auto &[device, values] : captureData)
    if (!std::ranges::contains(samplerDvcs, device))
      axisInfos.push_back({{device, values},
                           {"y [Volts]", ImAxis_Y1},
                           std::to_string(device.sampleRate)});
  return axisInfos;
}

void addPlots(const char *name,
              std::function<void(double, std::string, ImAxis_, double, double)>
                  axesSetup) {
  static std::set<std::string> firstRun;
  auto const plotRegion = ImGui::GetContentRegionAvail();
  // TODO search devices must work aswell
  plotAxes = getDeviceInfos();

  if (ImPlot::BeginPlot(name, plotRegion, ImPlotFlags_NoFrame)) {
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::min();

    for (auto const &axes : plotAxes) {
      x_max = std::max(x_max, axes.data.second.back().first);
      // TODO save max and min value over same axis
      // find first and last pairs of values
      auto [min, max] =
          std::minmax_element(axes.data.second.begin(), axes.data.second.end());
      double yMin = min->first + (min->first * 0.15);
      double yMax = max->second + (max->second * 0.15);
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

static void parseDeviceMetaData(Omniscope::MetaData metaData,
                                std::shared_ptr<OmniscopeDevice> &device) {
  try {
    nlohmann::json metaJson = metaData.data;
    fmt::println("metaJson content is: {}", metaJson.dump());
    device->setScale(metaJson["scale"]);
    device->setOffset(metaJson["offset"]);
    device->setEgu(metaJson["egu"]);
  } catch (...) {
    fmt::println("Parsing Meta Data error: {}", metaData.data);
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
    device->setTimeScale(1 / sampleRate);
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
      if (!flagPaused)
        doDevice(device.first, appLanguage[Key::Measurement]);
      else
        doDevice(device.first, appLanguage[Key::Stop]);
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

void set_config(std::string_view configpath) {
  if (fs::exists(configpath))
    fmt::println("Found config.json\r");
  else {
    fmt::println("Did not find config.json.\nDownload from Github\r");
    update_config_from_github();
  }
}

void set_json(const nlohmann::json &config) {
  if (fs::exists(load_json<std::string>(config, "languagepath")))
    fmt::println("Found language: {}\r", appLanguage[Key::German]);
  else {
    fmt::println("Did not find {}.\nDownload from Github\n",
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
