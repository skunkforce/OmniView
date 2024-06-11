#include "handler.hpp"

#include <implot.h>

#include <functional>
#include <set>

#include "get_from_github.hpp"

std::vector<AxisInfo> getDeviceInfos() {
    std::vector<AxisInfo> axisInfos;
    std::vector<std::pair<std::string, ImAxis_>> assignedEgus;
    if (sampler.has_value()) {
        for (auto const &device : sampler->sampleDevices) {
            // TODO replace ADC counts with language variable
            std::string egu = device.first->getEgu().value_or("ADC counts");
            auto id = device.first->getId();
            if (id.has_value()) {
                Omniscope::Id deviceId = id.value();
                std::string timebase{std::to_string(deviceId.sampleRate)};
                if (captureData.find(deviceId) != captureData.end()) {
                    auto eguIterator = std::ranges::find(
                        assignedEgus, egu,
                        &std::pair<std::string, ImAxis_>::first);
                    if (eguIterator == assignedEgus.end()) {
                        if (assignedEgus.size() <= 3) {
                            ImAxis_ nextYAxis = static_cast<ImAxis_>(
                                ImAxis_Y1 + assignedEgus.size());
                            assignedEgus.push_back(
                                std::make_pair(egu, nextYAxis));
                            eguIterator = (assignedEgus.end() - 1);
                        } else {
                            fmt::print(
                                "too many Axes added, egu not added: "
                                "{}\nDevice id: {}",
                                egu, id.value());
                            break;
                        }
                    }
                    AxisInfo axisInfo{
                        std::make_pair(deviceId,
                                       std::ref(captureData[deviceId])),
                        *eguIterator, timebase};
                    axisInfos.push_back(axisInfo);
                }
            } else {
                fmt::print("Error no device id found\n");
            }
        }
    }
    return axisInfos;
}

void addPlots(const char *name, bool const flagPaused,
              std::function<void(double, std::string, ImAxis_, double, double)>
                  axesSetup) {
    static std::set<std::string> firstRun;
    auto const plotRegion = ImGui::GetContentRegionAvail();
    static int activeAxes{0};
    //TODO search devices must work aswell
    if (plotAxes.size() <= activeAxes) {
        plotAxes = getDeviceInfos();
        activeAxes = plotAxes.size();

    }
    if (ImPlot::BeginPlot(name, plotRegion, ImPlotFlags_NoFrame)) {
        double x_min = std::numeric_limits<double>::max();
        double x_max = std::numeric_limits<double>::min();

        for (auto const &axes : plotAxes) {
            // fmt::print("data size:{}, egu: {}\n", axes.data.second.size(),
            //           axes.egu.first);
            if (!axes.data.second.empty()) {
                x_max = std::max(x_max, axes.data.second.back().first);
                //TODO save max and min value over same axis
                auto [min, max] = std::minmax_element(axes.data.second.begin(),
                                                      axes.data.second.end());
                double yMin = min->first + (min->first * 0.15);
                double yMax = max->second + (max->second * 0.15);
                // fmt::print("yMin {}, yMax{}\n", yMin, yMax);
                axesSetup(x_max, axes.egu.first, axes.egu.second, yMin, yMax);
            }
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
                    auto p = std::lower_bound(
                        plot.second.begin(), plot.second.end(),
                        std::pair<double, double>{limits.X.Min, 0});
                    if (p != plot.second.begin()) return p - 1;
                    return p;
                }();

                auto const end = [&]() {
                    auto p = std::upper_bound(
                        start, plot.second.end(),
                        std::pair<double, double>{limits.X.Max, 0});
                    if (p != plot.second.end()) return p + 1;
                    return p;
                }();

                std::size_t const stride = [&]() -> std::size_t {
                    auto const s =
                        std::distance(start, end) / (plotRegion.x * 2.0);
                    if (1 >= s) return 1;
                    return static_cast<std::size_t>(s);
                }();

                // determine which axes is the right one to choose
                ImPlot::SetAxes(ImAxis_X1, yAxis);
                ImPlot::PlotLine(
                    fmt::format("{}-{}", plot.first.type, plot.first.serial)
                        .c_str(),
                    std::addressof(start->first), std::addressof(start->second),
                    static_cast<std::size_t>(std::distance(start, end)) /
                        stride,
                    0, 0, 2 * sizeof(double) * stride);
            }
        };
        for (int count = 0; auto const &plot : plotAxes) {
            ImPlot::SetNextLineStyle(ImVec4{
                colorMap[plot.data.first][0], colorMap[plot.data.first][1],
                colorMap[plot.data.first][2], 1.0f});
            addPlot(plot.data, plot.egu.second);
        }

        ImPlot::EndPlot();
    }
}

void parseDeviceMetaData(Omniscope::MetaData metaData,
                         std::shared_ptr<OmniscopeDevice> &device) {
    try {
        nlohmann::json metaJson = nlohmann::json::parse(metaData.data);
        fmt::print("{}\n", metaJson.dump());
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
        device->send(
            Omniscope::SetRgb{static_cast<std::uint8_t>(color[0] * 255),
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
        ImGui::TextUnformatted(
            fmt::format("HW: v{}.{}.{} SW: v{}.{}.{}    ",
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
        for (auto &device : devices) doDevice(device, appLanguage[Key::Ready]);
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
    if (std::filesystem::exists(
            load_json<std::string>(config, ("languagepath"))))
        fmt::print("Found language: {}\n\r", appLanguage[Key::German]);
    else {
        fmt::print("Did not find {}.\n Download from Github\n\r",
                   appLanguage[Key::German]);
        update_language_from_github();
    }
}
void rstSettings() {
    sampler.reset();
    devices.clear();
    savedFileNames.clear();
    deviceManager.clearDevices();
    captureData.clear();
    plotAxes.clear();
}
