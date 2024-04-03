#include "style.hpp"
#include "imagesHeader.hpp"
#include "jasonhandler.hpp"
#define STB_IMAGE_IMPLEMENTATION
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
  static bool loaded_png{false};
  static int my_image_height{};
  static int my_image_width{};
  static GLuint my_image_texture{};
  const unsigned char *imagesNames{AutoInternLogo_png};
  static unsigned int imagesLen{AutoInternLogo_png_len};

  if (!loaded_png) {
    if (LoadTextureFromHeader(imagesNames, imagesLen, &my_image_texture,
                              &my_image_width, &my_image_height))
      loaded_png = true;
    else
      fmt::println("Error Loading Png.");
  }

  if (loaded_png)
    // render AIGroupLogo
    ImGui::Image((void *)(intptr_t)my_image_texture,
                 ImVec2(my_image_width * windowSize.x * 0.0005f,
                        my_image_height * windowSize.y * 0.0008f));

  // Start only if devices are available, otherwise search for devices
  if (flagPaused && !sampler.has_value() &&
      ImGui::Button("Search for devices")) {
    devices.clear();
    deviceManager.clearDevices();
    initDevices(deviceManager, devices, colorMap);
  }
}
