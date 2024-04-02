// style.hpp
#ifndef	STYLE_HEADER_HPP
#define STYLE_HEADER_HPP

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

inline constexpr ImVec4 inctColStyle {0.5f, 0.5f, 0.5f, 1.f};
inline constexpr ImVec4 normColStyle{0.1f, 0.1f, 0.1f, 1.f};
void SetupImGuiStyle(bool, float);
void set_button_style_to(const nlohmann::json&, const std::string&);
void load_settings(const nlohmann::json&);
void set_side_menu();
#endif