// style.hpp
#ifndef	STYLE_HEADER_HPP
#define STYLE_HEADER_HPP

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

void SetupImGuiStyle(bool, float);
void set_button_style_to(const nlohmann::json&, const std::string&);
void load_settings(const nlohmann::json&);

#endif