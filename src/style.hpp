// style.hpp
#ifndef STYLE_HEADER_HPP
#define STYLE_HEADER_HPP

#include "../ImGuiInstance/src/ImGuiInstance/ImGuiInstance.hpp"
#include "handler.hpp"

void set_side_menu(const nlohmann::json &, bool &, bool &,
                   decltype(captureData) &,
                   std::map<Omniscope::Id, std::string> &);
void set_toolbar(const nlohmann::json &, const nlohmann::json &, bool &,
                 const decltype(captureData) &);
#endif