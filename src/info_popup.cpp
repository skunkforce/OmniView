#include "languages.hpp"
#include "popups.hpp"
#include <imgui.h>

void info_popup(std::string_view title, std::string_view text) {
  if (ImGui::BeginPopupModal(title.data(), nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    ImGui::Text(text.data());
    if (ImGui::Button(appLanguage[Key::Back]))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

void info_popup_test(std::string title, std::string text) {
  if (ImGui::BeginPopupModal(title.data(), nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    ImGui::Text(text.c_str());
    if (ImGui::Button(appLanguage[Key::Back]))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}
