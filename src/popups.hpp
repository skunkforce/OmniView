// popups.hpp

#ifndef POPUPS_HEADER_HPP
#define POPUPS_HEADER_HPP

#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include "imgui_stdlib.h"
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeDevice.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"

void generateTrainingData(bool &,
                          const std::vector<std::shared_ptr<OmniscopeDevice>> &,
                          const std::vector<std::string> &);
void warning_popup(bool &, const std::string &);

#endif