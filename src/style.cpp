#define IMGUI_DEFINE_MATH_OPERATORS
#define STB_IMAGE_IMPLEMENTATION
#include "style.hpp"
#include "imagesHeader.hpp"
#include "imgui_internal.h"
#include "jasonhandler.hpp"
#include "stb_image.h"

void SetupImGuiStyle(bool bStyleDark_, float alpha_,
                     const nlohmann::json &config) {
  ImGuiStyle &style = ImGui::GetStyle();

  // light style from Pacôme Danhiez (user itamago)
  // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267

  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = load_json<float>(config, "text", "scale");
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text] = {0.f, 0.f, 0.f, 1.f};
  style.Colors[ImGuiCol_TextDisabled] = {0.6f, 0.6f, 0.6f, 1.f};
  style.Colors[ImGuiCol_WindowBg] = {1.f, 1.f, 1.f, 1.f};
  // style.Colors[ImGuiCol_ChildBg] = {0.1f, 0.1f, 0.1f, 1.f};
  style.Colors[ImGuiCol_Border] = {0.f, 0.f, 0.f, 0.39f};
  style.Colors[ImGuiCol_BorderShadow] = {1.f, 1.f, 1.f, 0.1f};
  style.Colors[ImGuiCol_FrameBg] = {1.f, 1.f, 1.f, 0.94f};
  style.Colors[ImGuiCol_FrameBgHovered] = {0.26f, 0.59f, 0.98f, 0.40f};
  style.Colors[ImGuiCol_FrameBgActive] = {0.26f, 0.59f, 0.98f, 0.67f};
  style.Colors[ImGuiCol_TitleBg] = {0.96f, 0.96f, 0.96f, 1.f};
  style.Colors[ImGuiCol_TitleBgCollapsed] = {1.f, 1.f, 1.f, 0.51f};
  style.Colors[ImGuiCol_TitleBgActive] = {0.82f, 0.82f, 0.82f, 1.f};
  style.Colors[ImGuiCol_ScrollbarBg] = {0.98f, 0.98f, 0.98f, 0.53f};
  style.Colors[ImGuiCol_ScrollbarGrab] = {0.69f, 0.69f, 0.69f, 1.f};
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = {0.59f, 0.59f, 0.59f, 1.f};
  style.Colors[ImGuiCol_ScrollbarGrabActive] = {0.49f, 0.49f, 0.49f, 1.f};
  style.Colors[ImGuiCol_PopupBg] = {0.86f, 0.86f, 0.86f, 0.99f};
  style.Colors[ImGuiCol_CheckMark] = {0.26f, 0.59f, 0.98f, 1.f};
  style.Colors[ImGuiCol_SliderGrab] = {0.24f, 0.52f, 0.88f, 1.f};
  style.Colors[ImGuiCol_SliderGrabActive] = {0.26f, 0.59f, 0.98f, 1.f};
  style.Colors[ImGuiCol_Button] = {0.26f, 0.59f, 0.98f, 0.4f};
  style.Colors[ImGuiCol_ButtonHovered] = {0.26f, 0.59f, 0.98f, 1.f};
  style.Colors[ImGuiCol_ButtonActive] = {0.06f, 0.53f, 0.98f, 1.f};
  style.Colors[ImGuiCol_Header] = {0.26f, 0.59f, 0.98f, 0.31f};
  style.Colors[ImGuiCol_HeaderHovered] = {0.26f, 0.59f, 0.98f, 0.8f};
  style.Colors[ImGuiCol_HeaderActive] = {0.26f, 0.59f, 0.98f, 1.f};
  // style.Colors[ImGuiCol_Column] = {0.39f, 0.39f, 0.39f, 1.00f};
  // style.Colors[ImGuiCol_ColumnHovered] = {0.26f, 0.59f, 0.98f, 0.78f};
  // style.Colors[ImGuiCol_ColumnActive] = {0.26f, 0.59f, 0.98f, 1.f};
  style.Colors[ImGuiCol_ResizeGrip] = {1.f, 1.f, 1.f, 0.5f};
  style.Colors[ImGuiCol_ResizeGripHovered] = {0.26f, 0.59f, 0.98f, 0.67f};
  style.Colors[ImGuiCol_ResizeGripActive] = {0.26f, 0.59f, 0.98f, 0.95f};
  // style.Colors[ImGuiCol_CloseButton] = {0.59f, 0.59f, 0.59f, 0.50f};
  // style.Colors[ImGuiCol_CloseButtonHovered] = {0.98f, 0.39f, 0.36f, 1.f};
  // style.Colors[ImGuiCol_CloseButtonActive] = {0.98f, 0.39f, 0.36f, 1.f};
  style.Colors[ImGuiCol_PlotLines] = {0.39f, 0.39f, 0.39f, 1.f};
  style.Colors[ImGuiCol_PlotLinesHovered] = {1.f, 0.43f, 0.35f, 1.f};
  style.Colors[ImGuiCol_PlotHistogram] = {0.9f, 0.7f, 0.f, 1.f};
  style.Colors[ImGuiCol_PlotHistogramHovered] = {1.f, 0.6f, 0.f, 1.f};
  style.Colors[ImGuiCol_TextSelectedBg] = {0.26f, 0.59f, 0.98f, 0.35f};
  // style.Colors[ImGuiCol_ModalWindowDarkening] = {0.2f, 0.2f, 0.2f, 0.35f};

  if (bStyleDark_)
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      float H, S, V;
      ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
      if (S < 0.1f)
        V = 1.0f - V;
      ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
      if (col.w < 1.00f)
        col.w *= alpha_;
    }
  else
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

void set_side_menu(const bool &flagPaused,
                   const std::optional<OmniscopeSampler> &sampler,
                   std::vector<std::shared_ptr<OmniscopeDevice>> &devices,
                   OmniscopeDeviceManager &deviceManager,
                   std::map<Omniscope::Id, std::array<float, 3>> &colorMap) {

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
      AutoInternLogo_png, SearchDevices_png, Diagnostics_png, Settings_png,
      Help_png};
  static const unsigned int imagesLen[] = {
      AutoInternLogo_png_len, SearchDevices_png_len, Diagnostics_png_len,
      Settings_png_len, Help_png_len};
  // Load the images for the SideBarMenu
  for (size_t i = 0; i < size; i++)
    if (!loaded_png[i]) {
      if (LoadTextureFromHeader(imagesNames[i], imagesLen[i], &image_texture[i],
                                &image_width[i], &image_height[i]))
        loaded_png[i] = true;
      else
        fmt::println("Error Loading Png #{}.", i);
    }

  // Begin the SideBarMenu
  if (loaded_png[PngRenderedCnt]) { // render AIGroupLogo
    ImGui::Image((void *)(intptr_t)image_texture[PngRenderedCnt],
                 ImVec2(image_width[PngRenderedCnt] * windowSize.x * 0.0005,
                        image_height[PngRenderedCnt] * windowSize.y * 0.0008));
  }
  ImGui::Dummy(
      {0.f, windowSize.y * .2f}); // space between logo and menu buttons

  // Start only if devices are available, otherwise search for devices
  if (loaded_png[++PngRenderedCnt] && // render search for Devices
      !sampler.has_value() &&
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt],
          "Search for devices")) {
    devices.clear();
    deviceManager.clearDevices();
    initDevices(deviceManager, devices, colorMap);
  }
  if (loaded_png[++PngRenderedCnt] && // render Diagnostics
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt], "Diagnostics")) {
  }
  if (loaded_png[++PngRenderedCnt] && // render Settings
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt], "Settings")) {
  }
  if (loaded_png[++PngRenderedCnt] && // render Help
      ImGui::ImageButtonWithText(
          (void *)(intptr_t)image_texture[PngRenderedCnt], "Help")) {
  }
}
