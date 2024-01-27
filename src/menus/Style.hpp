
#include <ImGuiInstance/ImGuiInstance.hpp>

#include "../jasonhandler.hpp"

#include <../nlohmann/json.hpp>
#include <../nlohmann/json_fwd.hpp>
#include <imgui.h>

namespace ImGui {
inline void SetupImGuiStyle(bool bStyleDark_, float alpha_) {
  ImGuiStyle &style = ImGui::GetStyle();

  // light style from Pacôme Danhiez (user itamago)
  // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text] =
      ImVec4(248 / 255.0f, 249 / 255.0f, 250 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TextDisabled] =
      ImVec4(248 / 255.0f, 249 / 255.0f, 250 / 255.0f, 98 / 100.0f);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  // style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f,
  // 0.00f);
  style.Colors[ImGuiCol_PopupBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_Border] =
      ImVec4(235 / 255.0f, 62 / 255.0f, 54 / 255.0f, 98 / 100.0f);
  style.Colors[ImGuiCol_BorderShadow] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 98 / 100.0f);

  style.Colors[ImGuiCol_FrameBg] = ImVec4(
      23 / 255.0f, 23 / 255.0f, 20 / 255.0f,
      100 / 100.0f); // changes the color of the frame bg for the plot window
  style.Colors[ImGuiCol_FrameBgHovered] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_FrameBgActive] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TitleBg] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TitleBgActive] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_MenuBarBg] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ScrollbarBg] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ScrollbarGrab] =
      ImVec4(248 / 255.0f, 249 / 255.0f, 250 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(240 / 255.0f, 240 / 255.0f, 240 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_CheckMark] =
      ImVec4(235 / 255.0f, 62 / 255.0f, 54 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_SliderGrab] =
      ImVec4(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_SliderGrabActive] =
      ImVec4(240 / 255.0f, 240 / 255.0f, 240 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_Button] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ButtonHovered] =
      ImVec4(240 / 255.0f, 62 / 255.0f, 54 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ButtonActive] =
      ImVec4(235 / 255.0f, 62 / 255.0f, 54 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f,
  // 0.39f, 1.00f); style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f,
  // 0.59f, 0.98f, 0.78f); style.Colors[ImGuiCol_ColumnActive]          =
  // ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  // style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f,
  // 0.50f); style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f,
  // 0.36f, 1.00f); style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f,
  // 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  // style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f,
  // 0.35f);

  if (bStyleDark_) {
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      float H, S, V;
      ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

      if (S < 0.1f) {
        V = 1.0f - V;
      }
      ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
      if (col.w < 1.00f) {
        col.w *= alpha_;
      }
    }
  } else {
    // std::cout << ImGuiCol_COUNT << std::endl;
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      if (col.w < 1.00f) {
        col.x *= alpha_;
        col.y *= alpha_;
        col.z *= alpha_;
        col.w *= alpha_;
      }
    }
  }
}
} // namespace ImGui