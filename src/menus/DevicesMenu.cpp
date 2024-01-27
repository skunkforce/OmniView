#include <ImGuiInstance/ImGuiInstance.hpp>

void SetDevicesMenu(std::map<Omniscope::Id, std::array<float, 3>> &colorMap,
                    std::optional<OmniscopeSampler> &sampler,
                    std::vector<std::shared_ptr<OmniscopeDevice>> &devices) {

  // ############################ Devicelist
  // ##############################
  ImGui::BeginChild("Devicelist", ImVec2(-1, 0));
  // ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  // ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
  //                       ImVec2(0.5f, 0.5f));

  ImGui::Text("devices found:");
  if (ImGui::BeginListBox("##deviceListBox", ImVec2(1024, -1))) {
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
        ImGui::TextUnformatted(fmt::format("Error").c_str());
    };

    if (sampler.has_value()) {
      for (auto &device : sampler->sampleDevices)
        doDevice(device.first, "Messung");
    } else {
      for (auto &device : devices)
        doDevice(device, "Ready");
    }
    ImGui::EndListBox();
  }
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::End();
  ImGui::PopStyleColor(7);
}