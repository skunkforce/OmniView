#include <implot.h>
#include <set>
#include "handler.hpp"
#include "get_from_github.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"
#include "../imgui-stdlib/imgui_stdlib.h"

void addPlots(const char *name,
              std::function<void(double)> axesSetup) {
  static std::set<std::string> firstRun;
  const auto &plots{captureData};
  auto const plotRegion = ImGui::GetContentRegionAvail();

  if (ImPlot::BeginPlot(name, plotRegion, ImPlotFlags_NoFrame)) {
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::min();

    for (auto const &plot : plots)
      if (!plot.second.empty()) {
        x_min = std::min(x_min, plot.second.front().first);
        x_max = std::max(x_max, plot.second.back().first);
      }

    axesSetup(x_max);
    auto const limits = [&]() {
      if (!firstRun.contains(name)) {
        firstRun.insert(name);
        return ImPlotRect(x_min, x_max, 0, 0);
      }
      return ImPlot::GetPlotLimits();
    }();
    auto addPlot = [&](auto const &plot) {
      if (!plot.second.empty()) {
        auto const start = [&]() {
          auto p = std::lower_bound(plot.second.begin(), plot.second.end(),
                                    std::pair<double, double>{limits.X.Min, 0});
          if (p != plot.second.begin())
            return p - 1;
          return p;
        }();

        auto const end = [&]() {
          auto p = std::upper_bound(start, plot.second.end(),
                                    std::pair<double, double>{limits.X.Max, 0});
          if (p != plot.second.end())
            return p + 1;
          return p;
        }();

        std::size_t const stride = [&]() -> std::size_t {
          auto const s = std::distance(start, end) / (plotRegion.x * 2.0);
          if (1 >= s)
            return 1;
          return static_cast<std::size_t>(s);
        }();

        ImPlot::PlotLine(
            fmt::format("{}-{}", plot.first.type, plot.first.serial).c_str(),
            std::addressof(start->first), std::addressof(start->second),
            static_cast<std::size_t>(std::distance(start, end)) / stride, 0, 0,
            2 * sizeof(double) * stride);
      }
    };

    for (auto const &plot : plots) {
      ImPlot::SetNextLineStyle(ImVec4{colorMap[plot.first][0],
                                      colorMap[plot.first][1],
                                      colorMap[plot.first][2], 1.0f});
      addPlot(plot);
    }

    ImPlot::EndPlot();
  }
}

void initDevices() {
  constexpr int VID = 0x2e8au;
  constexpr int PID = 0x000au;

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
    device->send(Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
                                   static_cast<std::uint8_t>(color[1] * 255),
                                   static_cast<std::uint8_t>(color[2] * 255)});
  }
}

void devicesList() {
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
    for (auto &device : sampler->sampleDevices)
      doDevice(device.first, appLanguage[Key::Measurement]);
  else
    for (auto &device : devices)
      doDevice(device, appLanguage[Key::Ready]);
}

void load_files(decltype(captureData) &loadedDvcs,
                std::map<Omniscope::Id, std::string> &Dvcs_filenames, bool& loadFile) {
  static std::set<fs::path> loadedFileNames;
  auto do_load = [&loadedDvcs, &Dvcs_filenames] {
    pairDvc loadedDvc{};
    for (const auto &path : loadedFileNames) {
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
            loadedDvc.first.type = substr;
          if (i == 4) // fifth element (serial of scope)
            loadedDvc.first.serial = substr;
        }
        size_t indx{2};           // y_values start from line 2 of the file
        while (!readfile.eof()) { // fill the vector of the values
          double value{};
          readfile >> value;
          loadedDvc.second.emplace_back(indx++, value);
          static constexpr size_t bigNumber{10'000'000};
          readfile.ignore(bigNumber,
                          '\n'); // new line separator between elements
        }
        readfile.close();
        // pop the extra last element
        loadedDvc.second.pop_back();
        loadedDvcs.emplace(loadedDvc);
        Dvcs_filenames.emplace(loadedDvc.first, path.filename().string());
      }
    }
  };

  if (ImGui::BeginPopupModal(appLanguage[Key::Load_file_data], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    static ImGui::FileBrowser fileBrowser;
    static std::vector<std::string> pathArr{""};
    for (auto& path : pathArr) {
      ImGui::PushID(&path);
      ImGui::InputTextWithHint("##", appLanguage[Key::Path], &path);
      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Browse]))
        fileBrowser.Open();
      fileBrowser.Display();
      if (fileBrowser.HasSelected()) {
        loadedFileNames.emplace(fileBrowser.GetSelected()); // absolute path
        path = fileBrowser.GetSelected().string();
        fileBrowser.ClearSelected();
      } else if (!path.empty())
        loadedFileNames.emplace(path);
      ImGui::PopID();
    }
    if (ImGui::Button(" + "))
      pathArr.emplace_back("");
    ImGui::SetItemTooltip(appLanguage[Key::Load_another_file]);
    if (ImGui::Button(appLanguage[Key::Back])) {
      loadFile = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(appLanguage[Key::Load_file])) {
      do_load();
      ImGui::CloseCurrentPopup();
      loadFile = false;
    }
    ImGui::EndPopup();
  }
}
void set_config(const std::string &configpath) {
  if (std::filesystem::exists(configpath))
    fmt::print("found config.json\n\r");
  else {
    fmt::print("Did not find config.json.\n Download from Github\n\r");
    update_config_from_github();
  }
}
void set_json(nlohmann::json &config) {
  if (std::filesystem::exists(load_json<std::string>(config, ("languagepath"))))
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

void rstSettings(const decltype(captureData) &loadedDvcs) {
  sampler.reset();
  devices.clear();
  savedFileNames.clear();
  deviceManager.clearDevices();
  // erase all elements excpet loadedDvcs
  for (auto it = captureData.begin(); it != captureData.end();) {
    if (!loadedDvcs.contains(it->first))
      it = captureData.erase(it);
    else
      ++it;
  }
}
