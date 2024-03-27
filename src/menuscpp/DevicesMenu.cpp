// Devices Menu where the connected devices are listed
#include <ImGuiInstance/ImGuiInstance.hpp>

namespace DevicesRegion {

    void SetDevicesMenu(std::map<Omniscope::Id, std::array<float, 3>>& colorMap,
        std::optional<OmniscopeSampler>& sampler,
        std::vector<std::shared_ptr<OmniscopeDevice>>& devices) {

        ImGuiStyle& style = ImGui::GetStyle();

        // ############################ Devicelist
        // ##############################
        auto windowSize{ ImGui::GetIO().DisplaySize };
        ImGui::SetCursorPos({ windowSize.x * 0.18f, windowSize.y * 0.695f });
        ImGui::BeginChild("Devicelist");

        // setting the next item to the top middle of the menu
        ImGui::SetCursorPos({ windowSize.x * 0.37f, windowSize.y * 0.025f });
        ImGui::Text("devices found:");

        style.Colors[ImGuiCol_FrameBg] = { 37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 1.f };
        if (ImGui::BeginListBox("##deviceListBox", { windowSize.x * 0.82f,
                                    windowSize.y * 0.18f })) {
            auto doDevice = [&](auto& device, auto msg) {
                auto& color = colorMap[device->getId().value()];
                if (ImGui::ColorEdit3(
                    fmt::format("{:<32}",
                        fmt::format("{}-{}", device->getId().value().type,
                            device->getId().value().serial))
                    .c_str(),
                    color.data(),
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker |
                    ImGuiColorEditFlags_NoTooltip)) {
                    device->send(
                        Omniscope::SetRgb{ static_cast<std::uint8_t>(color[0] * 255),
                                          static_cast<std::uint8_t>(color[1] * 255),
                                          static_cast<std::uint8_t>(color[2] * 255) });
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
                    ImGui::TextUnformatted(fmt::format("Error").c_str());
                };

            if (sampler.has_value())
                for (auto& device : sampler->sampleDevices)
                    doDevice(device.first, "Messung");
            else
                for (auto& device : devices)
                    doDevice(device, "Ready");
            ImGui::EndListBox();
        }
        ImGui::EndChild();
    }
} // namespace DevicesRegion