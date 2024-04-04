#include "style.hpp"
#include "imagesHeader.hpp"
#include "jasonhandler.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>
#include <string>

void SetupImGuiStyle(bool bStyleDark_, float alpha_,
                     const nlohmann::json &config) {
  ImGuiStyle &style = ImGui::GetStyle();

  ImGuiIO &io = ImGui::GetIO();

  io.FontGlobalScale = load_json<float>(config, "text", "scale");
    
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text] =
      ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_TextDisabled] =
      ImVec4(248 / 255.0f, 249 / 255.0f, 250 / 255.0f, 98 / 100.0f);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_ChildBg]         = ImVec4(0.00f, 0.00f, 0.00f,
   0.00f);
  style.Colors[ImGuiCol_PopupBg] =
      ImVec4(37 / 255.0f, 40 / 255.0f, 43 / 255.0f, 100 / 100.0f);
  style.Colors[ImGuiCol_Border] =
      ImVec4(235 / 255.0f, 62 / 255.0f, 54 / 255.0f, 98 / 100.0f);
  style.Colors[ImGuiCol_BorderShadow] =
      ImVec4(23 / 255.0f, 23 / 255.0f, 20 / 255.0f, 98 / 100.0f);

  style.Colors[ImGuiCol_FrameBg] = ImVec4(
      23 / 255.0f, 23 / 255.0f, 20 / 255.0f,
      100 / 100.0f);// changes the color of the frame bg for the plot window
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
  style.Colors[ImGuiCol_Separator]                = ImVec4(0.39f, 0.39f,
   0.39f, 1.00f); style.Colors[ImGuiCol_SeparatorHovered]         = ImVec4(0.26f,
   0.59f, 0.98f, 0.78f); style.Colors[ImGuiCol_SeparatorActive]          =
  ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
   style.Colors[ImGuiCol_Tab]           = ImVec4(0.59f, 0.59f, 0.59f,
   0.50f); style.Colors[ImGuiCol_TabHovered]    = ImVec4(0.98f, 0.39f,
   0.36f, 1.00f); style.Colors[ImGuiCol_TabActive]     = ImVec4(0.98f,
  0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_TabUnfocused]     = ImVec4(0.98f,
  0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.98f,
  0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  style.Colors[ImGuiCol_DragDropTarget]  = ImVec4(0.20f, 0.20f, 0.20f,
   0.35f);

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

  style.Colors[ImGuiCol_Text] = { 0.0f, 0.0f, 0.0f, 1.0f };
  style.Colors[ImGuiCol_WindowBg] = { 1.0f, 1.0f, 0.93f, 1.0f };
  style.Colors[ImGuiCol_Border] = { 0.14f, 0.15f, 0.17f, 1.0f };
  style.Colors[ImGuiCol_FrameBg] = { 1.0f, 1.0f, 1.0f, 1.0f };
  style.Colors[ImGuiCol_FrameBgHovered] = { 1.0f, 1.0f, 1.0f, 1.0f };
  style.Colors[ImGuiCol_FrameBgActive] = { 1.0f, 1.0f, 1.0f, 1.0f };

  // colors when hovering and clicking the axes
  style.Colors[ImGuiCol_ButtonHovered] = { 0.94f, 0.94f, 0.94f, 1.0f };
  style.Colors[ImGuiCol_ButtonActive] = { 0.94f, 0.94f, 0.94f, 1.0f };
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


// For Development 
void PopupStyleEditor(bool bStyleDark, float alpha, nlohmann::json& config) {
    ImGui::Begin("Style Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize);

    ImGuiStyle& style = ImGui::GetStyle();

    // Add more sliders or controls for other parameters as needed

    ImGui::Separator();

  std::vector<StyleElement> styleElements = {
    {"Text", &style.Colors[ImGuiCol_Text], ImGuiCol_Text},
    {"Text Disabled", &style.Colors[ImGuiCol_TextDisabled], ImGuiCol_TextDisabled},
    {"Window Background", &style.Colors[ImGuiCol_WindowBg], ImGuiCol_WindowBg},
    {"Child Window Background", &style.Colors[ImGuiCol_ChildBg], ImGuiCol_ChildBg},
    {"Popup Background", &style.Colors[ImGuiCol_PopupBg], ImGuiCol_PopupBg},
    {"Border", &style.Colors[ImGuiCol_Border], ImGuiCol_Border},
    {"Border Shadow", &style.Colors[ImGuiCol_BorderShadow], ImGuiCol_BorderShadow},
    {"Frame Background", &style.Colors[ImGuiCol_FrameBg], ImGuiCol_FrameBg},
    {"Frame Background Hovered", &style.Colors[ImGuiCol_FrameBgHovered], ImGuiCol_FrameBgHovered},
    {"Frame Background Active", &style.Colors[ImGuiCol_FrameBgActive], ImGuiCol_FrameBgActive},
    {"Title Background", &style.Colors[ImGuiCol_TitleBg], ImGuiCol_TitleBg},
    {"Title Background Collapsed", &style.Colors[ImGuiCol_TitleBgCollapsed], ImGuiCol_TitleBgCollapsed},
    {"Title Background Active", &style.Colors[ImGuiCol_TitleBgActive], ImGuiCol_TitleBgActive},
    {"Menu Bar Background", &style.Colors[ImGuiCol_MenuBarBg], ImGuiCol_MenuBarBg},
    {"Scrollbar Background", &style.Colors[ImGuiCol_ScrollbarBg], ImGuiCol_ScrollbarBg},
    {"Scrollbar Grab", &style.Colors[ImGuiCol_ScrollbarGrab], ImGuiCol_ScrollbarGrab},
    {"Scrollbar Grab Hovered", &style.Colors[ImGuiCol_ScrollbarGrabHovered], ImGuiCol_ScrollbarGrabHovered},
    {"Scrollbar Grab Active", &style.Colors[ImGuiCol_ScrollbarGrabActive], ImGuiCol_ScrollbarGrabActive},
    {"Check Mark", &style.Colors[ImGuiCol_CheckMark], ImGuiCol_CheckMark},
    {"Slider Grab", &style.Colors[ImGuiCol_SliderGrab], ImGuiCol_SliderGrab},
    {"Slider Grab Active", &style.Colors[ImGuiCol_SliderGrabActive], ImGuiCol_SliderGrabActive},
    {"Button", &style.Colors[ImGuiCol_Button], ImGuiCol_Button},
    {"Button Hovered", &style.Colors[ImGuiCol_ButtonHovered], ImGuiCol_ButtonHovered},
    {"Button Active", &style.Colors[ImGuiCol_ButtonActive], ImGuiCol_ButtonActive},
    {"Header", &style.Colors[ImGuiCol_Header], ImGuiCol_Header},
    {"Header Hovered", &style.Colors[ImGuiCol_HeaderHovered], ImGuiCol_HeaderHovered},
    {"Header Active", &style.Colors[ImGuiCol_HeaderActive], ImGuiCol_HeaderActive},
    {"Separator", &style.Colors[ImGuiCol_Separator], ImGuiCol_Separator},
    {"Separator Hovered", &style.Colors[ImGuiCol_SeparatorHovered], ImGuiCol_SeparatorHovered},
    {"Separator Active", &style.Colors[ImGuiCol_SeparatorActive], ImGuiCol_SeparatorActive},
    {"Resize Grip", &style.Colors[ImGuiCol_ResizeGrip], ImGuiCol_ResizeGrip},
    {"Resize Grip Hovered", &style.Colors[ImGuiCol_ResizeGripHovered], ImGuiCol_ResizeGripHovered},
    {"Resize Grip Active", &style.Colors[ImGuiCol_ResizeGripActive], ImGuiCol_ResizeGripActive},
    {"Tab", &style.Colors[ImGuiCol_Tab], ImGuiCol_Tab},
    {"Tab Hovered", &style.Colors[ImGuiCol_TabHovered], ImGuiCol_TabHovered},
    {"Tab Active", &style.Colors[ImGuiCol_TabActive], ImGuiCol_TabActive},
    {"Tab Unfocused", &style.Colors[ImGuiCol_TabUnfocused], ImGuiCol_TabUnfocused},
    {"Tab Unfocused Active", &style.Colors[ImGuiCol_TabUnfocusedActive], ImGuiCol_TabUnfocusedActive},
    {"Plot Lines", &style.Colors[ImGuiCol_PlotLines], ImGuiCol_PlotLines},
    {"Plot Lines Hovered", &style.Colors[ImGuiCol_PlotLinesHovered], ImGuiCol_PlotLinesHovered},
    {"Plot Histogram", &style.Colors[ImGuiCol_PlotHistogram], ImGuiCol_PlotHistogram},
    {"Plot Histogram Hovered", &style.Colors[ImGuiCol_PlotHistogramHovered], ImGuiCol_PlotHistogramHovered},
    {"Text Selected Background", &style.Colors[ImGuiCol_TextSelectedBg], ImGuiCol_TextSelectedBg},
    {"Drag Drop Target", &style.Colors[ImGuiCol_DragDropTarget], ImGuiCol_DragDropTarget}
};


    // Hier füge deinen Color Picker hinzu
    static ImVec4 copiedColors[ImGuiCol_COUNT];
    // Color pickers for individual style elements
    if (ImGui::TreeNode("Style Colors")) {
       UpdateColors(styleElements, copiedColors);
        // Add more color pickers for other style elements

        ImGui::TreePop();
    }

    ImGui::End();
}

void UpdateColor(ImVec4& color, ImGuiStyle& style, int styleIndex) {
    // Update color in the style
    style.Colors[styleIndex] = color;
}

void UpdateColors(const std::vector<StyleElement>& styleElements, const ImVec4* colors) {
   
  ImGuiStyle& style = ImGui::GetStyle();
  int i = 0; 
    for (const auto& element : styleElements) {
        ImGui::ColorEdit4(element.name, (float*)&colors[i]);
        ImVec4 colorWithAlpha = ImVec4(colors[i].x, colors[i].y, colors[i].z, style.Colors[element.colorIndex].w);
        style.Colors[element.colorIndex] = colorWithAlpha;
        i++; 
    }
}
