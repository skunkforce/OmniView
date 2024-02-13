#include "popups.hpp"

void warning_popup(std::string_view title, std::string_view text) {
  if (title == "Speicherwarnung")
    appLanguage = germanLan;
  else 
     appLanguage = englishLan;

  if (ImGui::BeginPopupModal(title.data(), nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    ImGui::Text(text.data());

    if (ImGui::Button(appLanguage["Back"]))
      ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }
}
