// style.hpp
#ifndef STYLE_HEADER_HPP
#define STYLE_HEADER_HPP

#include "../ImGuiInstance/src/ImGuiInstance/ImGuiInstance.hpp"
#include "handler.hpp"

struct StyleElement {
  const char *name;
  ImVec4 *color;
  ImGuiCol colorIndex;
};

inline constexpr ImVec4 inctColStyle{0.5f, 0.5f, 0.5f, 1.f};
inline constexpr ImVec4 normColStyle{0.1f, 0.1f, 0.1f, 1.f};
void SetupImGuiStyle(bool, float, const nlohmann::json &);
void set_button_style_to(const nlohmann::json &, const std::string &);
bool LoadTextureFromHeader(unsigned char const *, int, GLuint *, int *, int *);
void set_side_menu(const nlohmann::json &, bool &, bool &, bool &);

void PopupStyleEditor();

void PushPlotRegionColors(); 
void PopPlotRegionColors(); 

void SetDeviceMenuStyle();

namespace ImGui {
IMGUI_API bool ImageButtonWithText(ImTextureID texId, const char *label,
                                   const ImVec2 &imageSize = ImVec2(0, 0),
                                   const ImVec2 &uv0 = ImVec2(0, 0),
                                   const ImVec2 &uv1 = ImVec2(1, 1),
                                   int frame_padding = -1,
                                   const ImVec4 &bg_col = ImVec4(0, 0, 0, 0),
                                   const ImVec4 &tint_col = ImVec4(1, 1, 1, 1));
} // namespace ImGui

void SetHorizontalSepeareatorColours();
#endif