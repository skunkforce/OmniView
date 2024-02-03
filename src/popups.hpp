// popups.hpp

#ifndef POPUPS_HEADER_HPP
#define POPUPS_HEADER_HPP

#include "../ai_omniscope-v2-communication_sw/src/OmniscopeDevice.hpp"
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

void generateTrainingData(bool &,
                          const std::vector<std::shared_ptr<OmniscopeDevice>> &,
                          const std::vector<std::string> &);
void save_warning_popup(bool &, const std::string &);

#endif
