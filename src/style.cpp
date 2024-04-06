#define IMGUI_DEFINE_MATH_OPERATORS
#define STB_IMAGE_IMPLEMENTATION
#include "style.hpp"
#include "imagesHeader.hpp"
#include "imgui_internal.h"
#include "jasonhandler.hpp"
#include "languages.hpp"
#include "stb_image.h"
#include <cmake_git_version/version.hpp>

#include <string>
#include <vector>

void SetupImGuiStyle(bool bStyleDark_, float alpha_,
                     const nlohmann::json &config) {
  ImGuiStyle &style = ImGui::GetStyle();

  ImGuiIO &io = ImGui::GetIO();

  // io.FontGlobalScale = load_json<float>(config, "text", "scale");

  style.Alpha = 1.0f;
  style.FrameRounding = 5.0f;
  style.Colors[ImGuiCol_Text] =
      ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TextDisabled] =
      ImVec4(248 / 255.0f, 249 / 255.0f, 250 / 255.0f, 98 / 100.0f);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ChildBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_PopupBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_Border] =
      ImVec4(240 / 255.0f, 62 / 255.0f, 54 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

  style.Colors[ImGuiCol_FrameBg] = ImVec4(
      ImVec4(0.0f, 0.0f, 0.0f,
             1.0f)); // changes the color of the frame bg for the plot window
  style.Colors[ImGuiCol_FrameBgHovered] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_FrameBgActive] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TitleBg] =
      ImVec4(1 / 255.0f, 1 / 255.0f, 1 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TitleBgActive] =
      ImVec4(1 / 255.0f, 1 / 255.0f, 1 / 255.0f, 100 / 100.0f);
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
  style.Colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  style.Colors[ImGuiCol_Tab] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
  style.Colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_TabActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_TabUnfocusedActive] =
      ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

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

void set_button_style_to(const nlohmann::json &config,
                         const std::string &name) {
  ImGui::PushStyleColor(
      ImGuiCol_Button,
      ImVec4(load_json<Color>(config, "button", name, "normal")));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonHovered,
      ImVec4(load_json<Color>(config, "button", name, "hover")));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonActive,
      ImVec4(load_json<Color>(config, "button", name, "active")));
}

void SetMainWindowStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImPlotStyle &plotStyle = ImPlot::GetStyle();

  style.Colors[ImGuiCol_Text] = {0.0f, 0.0f, 0.0f, 1.0f};
  style.Colors[ImGuiCol_WindowBg] = {1.0f, 1.0f, 0.93f, 1.0f};
  style.Colors[ImGuiCol_ChildBg] =
      ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_Border] = {240 / 255.0f, 240 / 255.0f, 240 / 255.0f,
                                   100 / 100.0f};
  style.Colors[ImGuiCol_FrameBg] = {1.0f, 1.0f, 1.0f, 1.0f};
  // style.Colors[ImGuiCol_FrameBgHovered] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // style.Colors[ImGuiCol_FrameBgActive] = { 1.0f, 1.0f, 1.0f, 1.0f };

  // Colors for the Implot Window:
  plotStyle.Colors[ImPlotCol_PlotBg] = {1.0f, 1.0f, 1.0f, 1.0f};
  plotStyle.Colors[ImPlotCol_AxisBg] = {1.0f, 1.0f, 1.0f, 1.0f};
  plotStyle.Colors[ImPlotCol_AxisBgHovered] = {158.0 / 255.0f, 158.0 / 255.0f,
                                               158.0 / 255.0f, 1.0f};
  plotStyle.Colors[ImPlotCol_AxisBgActive] = {91.0 / 255.0f, 91.0 / 255.0f,
                                              91.0 / 255.0f, 1.0f};
  plotStyle.Colors[ImPlotCol_Line] = {1.0f, 1.0f, 1.0f, 1.0f};
  plotStyle.Colors[ImPlotCol_TitleText] = {0.0f, 0.0f, 0.0f, 1.0f};
  plotStyle.Colors[ImPlotCol_AxisGrid] = {0.0f, 0.0f, 0.0f, 1.0f};

  // colors when hovering and clicking the axes
  // style.Colors[ImGuiCol_ButtonHovered] = { 0.94f, 0.94f, 0.94f, 1.0f };
  // style.Colors[ImGuiCol_ButtonActive] = { 0.94f, 0.94f, 0.94f, 1.0f };

  // plotStyle.Colors[ImPlotCol_PlotBg]= { 1.0f, 1.0f, 0.93f, 1.0f };
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

void set_side_menu(const nlohmann::json &config, bool &flagPaused,
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

  float scaleWidth = ImGui::GetIO().DisplaySize.x * 0.0005;
  float scaleHeight = ImGui::GetIO().DisplaySize.y * 0.0008;
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
  const bool showSettingsPrev = showSettings;
  if (loaded_png[++PngRenderedCnt] && // render Settings
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt],
          appLanguage[Key::Attitude])) {
    showSettings = !showSettings;
  }
  if (showSettings && !showSettingsPrev)
    ImGui::SetNextItemOpen(false);
  if (showSettings && ImGui::TreeNode(appLanguage[Key::LanOption])) {
    if (ImGui::Button(appLanguage[Key::English])) {
      appLanguage = englishLan;
      showSettings = false;
    }
    if (ImGui::Button(appLanguage[Key::German])) {
      appLanguage = germanLan;
      showSettings = false;
    }
    ImGui::TreePop();
  }
  if (showSettings && ImGui::Button(appLanguage[Key::Settings])) {
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
  ImGui::SetCursorPosY(ImGui::GetIO().DisplaySize.y * 0.90f);
  ImGui::Text(fmt::format("{}: {}", appLanguage[Key::Version],
                          CMakeGitVersion::VersionWithGit)
                  .c_str());
}

// For Development
void PopupStyleEditor(bool bStyleDark, float alpha, nlohmann::json &config) {
  ImGui::Begin("Style Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  ImGuiStyle &style = ImGui::GetStyle();

  // Add more sliders or controls for other parameters as needed

  ImGui::Separator();

  std::vector<StyleElement> styleElements = {
      {"Text", &style.Colors[ImGuiCol_Text], ImGuiCol_Text},
      {"Text Disabled", &style.Colors[ImGuiCol_TextDisabled],
       ImGuiCol_TextDisabled},
      {"Window Background", &style.Colors[ImGuiCol_WindowBg],
       ImGuiCol_WindowBg},
      {"Child Window Background", &style.Colors[ImGuiCol_ChildBg],
       ImGuiCol_ChildBg},
      {"Popup Background", &style.Colors[ImGuiCol_PopupBg], ImGuiCol_PopupBg},
      {"Border", &style.Colors[ImGuiCol_Border], ImGuiCol_Border},
      {"Border Shadow", &style.Colors[ImGuiCol_BorderShadow],
       ImGuiCol_BorderShadow},
      {"Frame Background", &style.Colors[ImGuiCol_FrameBg], ImGuiCol_FrameBg},
      {"Frame Background Hovered", &style.Colors[ImGuiCol_FrameBgHovered],
       ImGuiCol_FrameBgHovered},
      {"Frame Background Active", &style.Colors[ImGuiCol_FrameBgActive],
       ImGuiCol_FrameBgActive},
      {"Title Background", &style.Colors[ImGuiCol_TitleBg], ImGuiCol_TitleBg},
      {"Title Background Collapsed", &style.Colors[ImGuiCol_TitleBgCollapsed],
       ImGuiCol_TitleBgCollapsed},
      {"Title Background Active", &style.Colors[ImGuiCol_TitleBgActive],
       ImGuiCol_TitleBgActive},
      {"Menu Bar Background", &style.Colors[ImGuiCol_MenuBarBg],
       ImGuiCol_MenuBarBg},
      {"Scrollbar Background", &style.Colors[ImGuiCol_ScrollbarBg],
       ImGuiCol_ScrollbarBg},
      {"Scrollbar Grab", &style.Colors[ImGuiCol_ScrollbarGrab],
       ImGuiCol_ScrollbarGrab},
      {"Scrollbar Grab Hovered", &style.Colors[ImGuiCol_ScrollbarGrabHovered],
       ImGuiCol_ScrollbarGrabHovered},
      {"Scrollbar Grab Active", &style.Colors[ImGuiCol_ScrollbarGrabActive],
       ImGuiCol_ScrollbarGrabActive},
      {"Check Mark", &style.Colors[ImGuiCol_CheckMark], ImGuiCol_CheckMark},
      {"Slider Grab", &style.Colors[ImGuiCol_SliderGrab], ImGuiCol_SliderGrab},
      {"Slider Grab Active", &style.Colors[ImGuiCol_SliderGrabActive],
       ImGuiCol_SliderGrabActive},
      {"Button", &style.Colors[ImGuiCol_Button], ImGuiCol_Button},
      {"Button Hovered", &style.Colors[ImGuiCol_ButtonHovered],
       ImGuiCol_ButtonHovered},
      {"Button Active", &style.Colors[ImGuiCol_ButtonActive],
       ImGuiCol_ButtonActive},
      {"Header", &style.Colors[ImGuiCol_Header], ImGuiCol_Header},
      {"Header Hovered", &style.Colors[ImGuiCol_HeaderHovered],
       ImGuiCol_HeaderHovered},
      {"Header Active", &style.Colors[ImGuiCol_HeaderActive],
       ImGuiCol_HeaderActive},
      {"Separator", &style.Colors[ImGuiCol_Separator], ImGuiCol_Separator},
      {"Separator Hovered", &style.Colors[ImGuiCol_SeparatorHovered],
       ImGuiCol_SeparatorHovered},
      {"Separator Active", &style.Colors[ImGuiCol_SeparatorActive],
       ImGuiCol_SeparatorActive},
      {"Resize Grip", &style.Colors[ImGuiCol_ResizeGrip], ImGuiCol_ResizeGrip},
      {"Resize Grip Hovered", &style.Colors[ImGuiCol_ResizeGripHovered],
       ImGuiCol_ResizeGripHovered},
      {"Resize Grip Active", &style.Colors[ImGuiCol_ResizeGripActive],
       ImGuiCol_ResizeGripActive},
      {"Tab", &style.Colors[ImGuiCol_Tab], ImGuiCol_Tab},
      {"Tab Hovered", &style.Colors[ImGuiCol_TabHovered], ImGuiCol_TabHovered},
      {"Tab Active", &style.Colors[ImGuiCol_TabActive], ImGuiCol_TabActive},
      {"Tab Unfocused", &style.Colors[ImGuiCol_TabUnfocused],
       ImGuiCol_TabUnfocused},
      {"Tab Unfocused Active", &style.Colors[ImGuiCol_TabUnfocusedActive],
       ImGuiCol_TabUnfocusedActive},
      {"Plot Lines", &style.Colors[ImGuiCol_PlotLines], ImGuiCol_PlotLines},
      {"Plot Lines Hovered", &style.Colors[ImGuiCol_PlotLinesHovered],
       ImGuiCol_PlotLinesHovered},
      {"Plot Histogram", &style.Colors[ImGuiCol_PlotHistogram],
       ImGuiCol_PlotHistogram},
      {"Plot Histogram Hovered", &style.Colors[ImGuiCol_PlotHistogramHovered],
       ImGuiCol_PlotHistogramHovered},
      {"Text Selected Background", &style.Colors[ImGuiCol_TextSelectedBg],
       ImGuiCol_TextSelectedBg},
      {"Drag Drop Target", &style.Colors[ImGuiCol_DragDropTarget],
       ImGuiCol_DragDropTarget}};

  std::vector<StyleElement> plotElements = {
      {"Plot Line/Outline", &ImPlot::GetStyle().Colors[ImPlotCol_Line],
       ImPlotCol_Line},
      {"Plot Fill", &ImPlot::GetStyle().Colors[ImPlotCol_Fill], ImPlotCol_Fill},
      {"Marker Outline", &ImPlot::GetStyle().Colors[ImPlotCol_MarkerOutline],
       ImPlotCol_MarkerOutline},
      {"Marker Fill", &ImPlot::GetStyle().Colors[ImPlotCol_MarkerFill],
       ImPlotCol_MarkerFill},
      {"Error Bar", &ImPlot::GetStyle().Colors[ImPlotCol_ErrorBar],
       ImPlotCol_ErrorBar},
      {"Frame Background", &ImPlot::GetStyle().Colors[ImPlotCol_FrameBg],
       ImPlotCol_FrameBg},
      {"Plot Background", &ImPlot::GetStyle().Colors[ImPlotCol_PlotBg],
       ImPlotCol_PlotBg},
      {"Plot Border", &ImPlot::GetStyle().Colors[ImPlotCol_PlotBorder],
       ImPlotCol_PlotBorder},
      {"Legend Background", &ImPlot::GetStyle().Colors[ImPlotCol_LegendBg],
       ImPlotCol_LegendBg},
      {"Legend Border", &ImPlot::GetStyle().Colors[ImPlotCol_LegendBorder],
       ImPlotCol_LegendBorder},
      {"Legend Text", &ImPlot::GetStyle().Colors[ImPlotCol_LegendText],
       ImPlotCol_LegendText},
      {"Title Text", &ImPlot::GetStyle().Colors[ImPlotCol_TitleText],
       ImPlotCol_TitleText},
      {"Inlay Text", &ImPlot::GetStyle().Colors[ImPlotCol_InlayText],
       ImPlotCol_InlayText},
      {"Axis Text", &ImPlot::GetStyle().Colors[ImPlotCol_AxisText],
       ImPlotCol_AxisText},
      {"Axis Grid", &ImPlot::GetStyle().Colors[ImPlotCol_AxisGrid],
       ImPlotCol_AxisGrid},
      {"Axis Tick", &ImPlot::GetStyle().Colors[ImPlotCol_AxisTick],
       ImPlotCol_AxisTick},
      {"Axis Background", &ImPlot::GetStyle().Colors[ImPlotCol_AxisBg],
       ImPlotCol_AxisBg},
      {"Axis Background Hovered",
       &ImPlot::GetStyle().Colors[ImPlotCol_AxisBgHovered],
       ImPlotCol_AxisBgHovered},
      {"Axis Background Active",
       &ImPlot::GetStyle().Colors[ImPlotCol_AxisBgActive],
       ImPlotCol_AxisBgActive},
      {"Selection", &ImPlot::GetStyle().Colors[ImPlotCol_Selection],
       ImPlotCol_Selection},
      {"Crosshairs", &ImPlot::GetStyle().Colors[ImPlotCol_Crosshairs],
       ImPlotCol_Crosshairs}};

  // Hier füge deinen Color Picker hinzu
  static ImVec4 copiedColors[ImGuiCol_COUNT];

  static ImVec4 plotcolors[ImPlotCol_COUNT];
  // Color pickers for individual style elements
  if (ImGui::TreeNode("Style Colors")) {
    UpdateColors(styleElements, copiedColors);
    // Add more color pickers for other style elements

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("ImPlotColors")) {
    UpdateColorsImPlot(plotElements, plotcolors);
    // Add more color pickers for other style elements

    ImGui::TreePop();
  }

  ImGui::End();
}

void UpdateColors(const std::vector<StyleElement> &styleElements,
                  const ImVec4 *colors) {

  ImGuiStyle &style = ImGui::GetStyle();
  int i = 0;
  for (const auto &element : styleElements) {
    ImGui::ColorEdit4(element.name, (float *)&colors[i]);
    ImVec4 colorWithAlpha = ImVec4(colors[i].x, colors[i].y, colors[i].z,
                                   style.Colors[element.colorIndex].w);
    style.Colors[element.colorIndex] = colorWithAlpha;
    i++;
  }
}

void UpdateColorsImPlot(const std::vector<StyleElement> &styleElements,
                        const ImVec4 *colors) {

  ImPlotStyle &plotStyle = ImPlot::GetStyle();
  int i = 0;
  for (const auto &element : styleElements) {
    ImGui::ColorEdit4(element.name, (float *)&colors[i]);
    ImVec4 colorWithAlpha =
        ImVec4(colors[i].x, colors[i].y, colors[i].z, 0.99f);
    plotStyle.Colors[element.colorIndex] = colorWithAlpha;
    i++;
  }
}
