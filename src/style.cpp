#define IMGUI_DEFINE_MATH_OPERATORS
#define STB_IMAGE_IMPLEMENTATION
#include "style.hpp"
#include "../stb_image/stb_image.h"
#include "imagesHeader.hpp"
#include "imgui_internal.h"
#include "jasonhandler.hpp"
#include "languages.hpp"
#include <cmake_git_version/version.hpp>
#include <iostream>
#include <string>
#include <vector>

void SetupImGuiStyle(bool bStyleDark_, float alpha_) {

  // ImGuiIO &io = ImGui::GetIO();
  // io.FontGlobalScale = load_json<float>(config, "text", "scale");

  ImGui::GetStyle().Alpha = 1.0f;
  ImGui::GetStyle().FrameRounding = 5.0f;

  auto colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = {1.f, 1.f, 1.f, 1.f};
  colors[ImGuiCol_TextDisabled] = {0.972f, 0.976f, 0.98f, 0.98f};
  colors[ImGuiCol_WindowBg] = {0.145f, 0.157f, 0.169f, 1.f};
  colors[ImGuiCol_ChildBg] = {0.145f, 0.157f, 0.169f, 1.f};
  colors[ImGuiCol_PopupBg] = {0.145f, 0.157f, 0.169f, 1.f};
  colors[ImGuiCol_Border] = {0.94f, 0.243f, 0.212f, 1.0f};
  colors[ImGuiCol_BorderShadow] = {0.f, 0.f, 0.f, 1.f};
  // changes the color of the frame bg for the plot window
  colors[ImGuiCol_FrameBg] = {0.f, 0.f, 0.f, 1.f};
  colors[ImGuiCol_FrameBgHovered] = {0.09f, 0.09f, 0.078f, 1.f};
  colors[ImGuiCol_FrameBgActive] = {0.09f, 0.09f, 0.078f, 1.f};
  colors[ImGuiCol_TitleBg] = {0.004f, 0.004f, 0.004f, 1.f};
  colors[ImGuiCol_TitleBgCollapsed] = {0.09f, 0.09f, 0.078f, 1.f};
  colors[ImGuiCol_TitleBgActive] = {0.004f, 0.004f, 0.004f, 1.f};
  colors[ImGuiCol_MenuBarBg] = {0.09f, 0.09f, 0.078f, 1.f};
  colors[ImGuiCol_ScrollbarBg] = {0.09f, 0.09f, 0.078f, 1.f};
  colors[ImGuiCol_ScrollbarGrab] = {0.972f, 0.976f, 0.98f, 0.98f};
  colors[ImGuiCol_ScrollbarGrabHovered] = {0.96f, 0.96f, 0.96f, 1.f};
  colors[ImGuiCol_ScrollbarGrabActive] = {0.941f, 0.941f, 0.941f, 1.f};
  colors[ImGuiCol_CheckMark] = {0.92f, 0.24f, 0.211f, 1.f};
  colors[ImGuiCol_SliderGrab] = {0.96f, 0.96f, 0.96f, 1.f};
  colors[ImGuiCol_SliderGrabActive] = {0.941f, 0.941f, 0.941f, 1.f};
  colors[ImGuiCol_Button] = {0.145f, 0.156f, 0.168f, 1.f};
  colors[ImGuiCol_ButtonHovered] = {0.941f, 0.243f, 0.211f, 1.f};
  colors[ImGuiCol_ButtonActive] = {0.921f, 0.24f, 0.211f, 1.f};
  colors[ImGuiCol_Header] = {0.26f, 0.59f, 0.98f, 0.31f};
  colors[ImGuiCol_HeaderHovered] = {0.26f, 0.59f, 0.98f, 0.80f};
  colors[ImGuiCol_HeaderActive] = {0.26f, 0.59f, 0.98f, 1.f};
  colors[ImGuiCol_Separator] = {0.39f, 0.39f, 0.39f, 1.f};
  colors[ImGuiCol_SeparatorHovered] = {0.26f, 0.59f, 0.98f, 0.78f};
  colors[ImGuiCol_SeparatorActive] = {0.26f, 0.59f, 0.98f, 1.f};
  colors[ImGuiCol_ResizeGrip] = {1.00f, 1.00f, 1.00f, 0.50f};
  colors[ImGuiCol_ResizeGripHovered] = {0.26f, 0.59f, 0.98f, 0.67f};
  colors[ImGuiCol_ResizeGripActive] = {0.26f, 0.59f, 0.98f, 0.95f};
  colors[ImGuiCol_Tab] = {0.59f, 0.59f, 0.59f, 0.50f};
  colors[ImGuiCol_TabHovered] = {0.98f, 0.39f, 0.36f, 1.f};
  colors[ImGuiCol_TabActive] = {0.98f, 0.39f, 0.36f, 1.f};
  colors[ImGuiCol_TabUnfocused] = {0.98f, 0.39f, 0.36f, 1.f};
  colors[ImGuiCol_TabUnfocusedActive] = {0.98f, 0.39f, 0.36f, 1.00f};
  colors[ImGuiCol_PlotLines] = {0.39f, 0.39f, 0.39f, 1.f};
  colors[ImGuiCol_PlotLinesHovered] = {1.f, 0.43f, 0.35f, 1.f};
  colors[ImGuiCol_PlotHistogram] = {0.9f, 0.7f, 0.f, 1.f};
  colors[ImGuiCol_PlotHistogramHovered] = {1.f, 0.6f, 0.f, 1.f};
  colors[ImGuiCol_TextSelectedBg] = {0.26f, 0.59f, 0.98f, 0.35f};
  colors[ImGuiCol_DragDropTarget] = {0.2f, 0.2f, 0.2f, 0.35f};

  if (bStyleDark_) {
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      ImVec4 &col = colors[i];
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
      ImVec4 &col = colors[i];
      if (col.w < 1.00f) {
        col.x *= alpha_;
        col.y *= alpha_;
        col.z *= alpha_;
        col.w *= alpha_;
      }
    }
  }
}

void set_button_style_to(const nlohmann::json &config,
                         const std::string &name) {
  ImGui::PushStyleColor(ImGuiCol_Button,
                        load_json<Color>(config, "button", name, "normal"));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        load_json<Color>(config, "button", name, "hover"));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        load_json<Color>(config, "button", name, "active"));
}

void PushPlotRegionColors() {
  ImGui::PushStyleColor(ImGuiCol_Text, {0.f, 0.f, 0.f, 1.f});
  ImGui::PushStyleColor(ImGuiCol_WindowBg, {1.f, 1.f, 0.93f, 1.f});
  ImGui::PushStyleColor(ImGuiCol_ChildBg, {1.f, 1.f, 1.f, 1.f});
  ImGui::PushStyleColor(ImGuiCol_Border, {0.941f, 0.941f, 0.941f, 1.f});
  ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.f, 1.f, 1.f, 1.f});

  ImPlot::PushStyleColor(ImPlotCol_PlotBg, {1.f, 1.f, 1.f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_AxisBg, {1.f, 1.f, 1.f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_AxisBgHovered, {0.61f, 0.61f, 0.61f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_AxisBgActive, {0.36f, 0.36f, 0.36f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_Line, {1.f, 1.f, 1.f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_TitleText, {0.f, 0.f, 0.f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_AxisGrid, {0.f, 0.f, 0.f, 1.f});
  ImPlot::PushStyleColor(ImPlotCol_LegendBg, {1.0f, 1.0f, 1.0f, 1.0f});
  ImPlot::PushStyleColor(ImPlotCol_LegendBorder, {37/255.0f, 40/255.0f, 43/255.0f, 1.0f}); // Schwarz
}
void PopPlotRegionColors() {
  ImGui::PopStyleColor(5);
  ImPlot::PopStyleColor(7);
}
void SetDeviceMenuStyle() {

  ImGuiStyle &style = ImGui::GetStyle();
  style.Colors[ImGuiCol_Border] = {0.14f, 0.15f, 0.17f, 1.0f};
}

namespace ImGui {
bool ImageButtonWithText(ImTextureID texId, const char *label,
                         const ImVec2 &imageSize, const ImVec2 &uv0,
                         const ImVec2 &uv1, int frame_padding,
                         const ImVec4 &bg_col, const ImVec4 &tint_col) {
  ImGuiWindow *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImVec2 size = imageSize;
  if (size.x <= 0 && size.y <= 0) {
    size.x = size.y = ImGui::GetTextLineHeightWithSpacing();
  } else {
    if (size.x <= 0)
      size.x = size.y;
    else if (size.y <= 0)
      size.y = size.x;
    size *= window->FontWindowScale * ImGui::GetIO().FontGlobalScale;
  }

  ImGuiContext &g = *GImGui;
  const ImGuiStyle &style = g.Style;
  const ImGuiID id = window->GetID(label);
  const ImVec2 textSize = ImGui::CalcTextSize(label, NULL, true);
  const bool hasText = textSize.x > 0;
  const float innerSpacing =
      hasText ? ((frame_padding >= 0) ? (float)frame_padding
                                      : (style.ItemInnerSpacing.x))
              : 0.f;
  const ImVec2 padding =
      (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding)
                           : style.FramePadding;
  const ImVec2 totalSizeWithoutPadding(size.x + innerSpacing + textSize.x,
                                       size.y > textSize.y ? size.y
                                                           : textSize.y);
  const ImRect bb(window->DC.CursorPos,
                  window->DC.CursorPos + totalSizeWithoutPadding + padding * 2);
  ImVec2 start(0, 0);
  start = window->DC.CursorPos + padding;
  if (size.y < textSize.y)
    start.y += (textSize.y - size.y) * .5f;
  const ImRect image_bb(start, start + size);
  start = window->DC.CursorPos + padding;
  start.x += size.x + innerSpacing;
  if (size.y > textSize.y)
    start.y += (size.y - textSize.y) * .5f;
  ItemSize(bb);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered = false, held = false;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);

  // Render
  const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive
                                : hovered         ? ImGuiCol_ButtonHovered
                                                  : ImGuiCol_Button);
  RenderFrame(
      bb.Min, bb.Max, col, true,
      ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
  if (bg_col.w > 0.0f)
    window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max,
                                    GetColorU32(bg_col));

  window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1,
                             GetColorU32(tint_col));

  if (textSize.x > 0)
    ImGui::RenderText(start, label);
  return pressed;
}
} // namespace ImGui

bool LoadTextureFromHeader(unsigned char const *png_data, int png_data_len,
                           GLuint *out_texture, int *out_width,
                           int *out_height) {
  // Load from file
  int image_width{};
  int image_height{};
  unsigned char *image_data = stbi_load_from_memory(
      png_data, png_data_len, &image_width, &image_height, NULL, 4);
  if (image_data == NULL)
    return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

void set_side_menu(const nlohmann::json &config,
                   bool &open_settings, bool &open_generate_training_data) {

  auto windowSize{ImGui::GetIO().DisplaySize};
  // Initializing all variables for images
  static constexpr size_t size{5}; // number of pictures
  size_t PngRenderedCnt{};
  static bool loaded_png[size]{};
  static int image_height[size];
  static int image_width[size];
  static GLuint image_texture[size];

  // The order matters because of the counter for the images !!!
  static const unsigned char *imagesNames[] = {
      AutoInternLogo_png, RefreshIcon_png, DiagnosticIcon_png, SettingIcon_png,
      HelpIcon_png};
  static const unsigned int imagesLen[] = {
      AutoInternLogo_png_len, RefreshIcon_png_len, DiagnosticIcon_png_len,
      SettingIcon_png_len, HelpIcon_png_len};
  // Load the images for the SideBarMenu
  for (size_t i = 0; i < size; i++)
    if (!loaded_png[i]) {
      if (LoadTextureFromHeader(imagesNames[i], imagesLen[i], &image_texture[i],
                                &image_width[i], &image_height[i]))
        loaded_png[i] = true;
      else
        fmt::println("Error Loading Png #{}.", i);
    }

  float scaleWidth = ImGui::GetIO().DisplaySize.x * 0.0005f;
  float scaleHeight = ImGui::GetIO().DisplaySize.y * 0.0008f;
  // Begin the SideBarMenu
  if (loaded_png[PngRenderedCnt]) { // render AIGroupLogo
    ImGui::Image((void *)(intptr_t)image_texture[PngRenderedCnt],
                 ImVec2(image_width[PngRenderedCnt] * scaleWidth,
                        image_height[PngRenderedCnt] * scaleHeight));
  }
  ImGui::Dummy({0.f, windowSize.y * .05f});

  // Start only if devices are available, otherwise search for devices
  if (loaded_png[++PngRenderedCnt] && // render search for Devices
      !sampler.has_value() &&
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt],
          appLanguage[Key::Dvc_search])) {
    devices.clear();
    deviceManager.clearDevices();
    initDevices();
  }

  static bool showDiag = false;
  const bool showDiagPrev = showDiag;
  if (loaded_png[++PngRenderedCnt] && // render Diagnostics
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt],
          appLanguage[Key::Diagnostics])) {
    showDiag = !showDiag;
  }
  if (showDiag && !showDiagPrev)
    ImGui::SetNextItemOpen(false);
  if (showDiag && ImGui::TreeNode(appLanguage[Key::Battery_measure])) {
    ImGui::PushStyleColor(ImGuiCol_Text, inctColStyle);
    if (ImGui::Button(appLanguage[Key::Anlyz_crnt_waveform]))
      showDiag = false;
    ImGui::PopStyleColor();
    if (ImGui::Button(appLanguage[Key::Gnrt_trning_data])) {
      open_generate_training_data = true;
      showDiag = false;
    }
    ImGui::TreePop();
  }

  static bool showSettings = false;
  if (loaded_png[++PngRenderedCnt] && // render Settings
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt],
          appLanguage[Key::Attitude])) {
    open_settings = true;
    showSettings = false;
  }

  if (loaded_png[++PngRenderedCnt] && // render Help
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt],
          appLanguage[Key::Help])) {
    system(("start " + load_json<std::string>(config, "helplink")).c_str());
    showSettings = false;
  }
}

// For Development
void PopupStyleEditor() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImPlotStyle &styleImPlot = ImPlot::GetStyle();
  static std::vector<ImVec4> colorVec;
  static std::vector<ImVec4> plotColors;

  if (colorVec.empty() && plotColors.empty()) {
    for (const auto &element : style.Colors)
      colorVec.push_back(element);
    for (const auto &element : styleImPlot.Colors)
      plotColors.push_back(element);
  }

  if (ImGui::TreeNode("Style Colors")) {
    for (std::size_t i{0}; i < ImGuiCol_COUNT - 1; ++i) {
      ImGui::ColorEdit4(ImGui::GetStyleColorName(i), (float *)&colorVec[i]);
      style.Colors[i] = colorVec[i];
    }
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Plot Colors")) {
    for (size_t i = 0; i < ImPlotCol_COUNT; i++) {
      ImGui::ColorEdit4(ImPlot::GetStyleColorName(i), (float *)&plotColors[i]);
      styleImPlot.Colors[i] = plotColors[i];
    }
    ImGui::TreePop();
  }
}

void SetHorizontalSepeareatorColours() {

  ImGuiStyle &style = ImGui::GetStyle();
  style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
}