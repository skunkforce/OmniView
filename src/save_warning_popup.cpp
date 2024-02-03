#include "popups.hpp"
#include <iostream>

void save_warning_popup(bool &popup, const std::string &msg) {

  ImGui::OpenPopup("Save warning");

  if (ImGui::BeginPopupModal("Save warning", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    ImGui::Text(msg.c_str());

    if (ImGui::Button("Close")) {
      popup = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}
