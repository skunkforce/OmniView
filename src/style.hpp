// style.hpp
#ifndef STYLE_HEADER_HPP
#define STYLE_HEADER_HPP

#include "../ImGuiInstance/src/ImGuiInstance/ImGuiInstance.hpp"
#include "handler.hpp"

struct StyleElement {
    const char* name;
    ImVec4* color;
    ImGuiCol colorIndex;
};

inline constexpr ImVec4 inctColStyle{0.5f, 0.5f, 0.5f, 1.f};
inline constexpr ImVec4 normColStyle{0.1f, 0.1f, 0.1f, 1.f};
void SetupImGuiStyle(bool, float, const nlohmann::json &);
void set_button_style_to(const nlohmann::json &, const std::string &);
bool LoadTextureFromHeader(unsigned char const *, int, GLuint *, int *, int *);
void set_side_menu(const bool &, const std::optional<OmniscopeSampler> &,
                   std::vector<std::shared_ptr<OmniscopeDevice>> &,
                   OmniscopeDeviceManager &,
                   std::map<Omniscope::Id, std::array<float, 3>> &);
void PopupStyleEditor(bool bStyleDark, float alpha, nlohmann::json& config); 
void UpdateColors(const std::vector<StyleElement>& styleElements, const ImVec4* colors); 
void UpdateColorsImPlot(const std::vector<StyleElement>& styleElements, const ImVec4* colors); 

void SetMainWindowStyle(); 

#endif