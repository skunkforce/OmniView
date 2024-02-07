#include "popups.hpp"

void warning_popup(bool& popup, const std::string& msg) {

    ImGui::OpenPopup("Warning");

    if (ImGui::BeginPopupModal("Warning", nullptr,
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