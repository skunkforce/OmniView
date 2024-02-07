// popups.hpp

#ifndef POPUPS_HEADER_HPP
#define POPUPS_HEADER_HPP

#include <imgui.h>
#include <string>
#include <set>
#include <memory>
#include <algorithm>
#include "imgui_stdlib.h"
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeDevice.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"

  // Have address of bool for std::vector
    struct BoolWrapper {
      BoolWrapper() : b(false) {}
      BoolWrapper(bool _b) : b(_b) {}
      bool b;
    };
    
void generateTrainingData(bool&, 
                    const std::vector<std::shared_ptr<OmniscopeDevice>>&,
                    std::set<std::string>&);
void warning_popup(bool&, const std::string&); 

#endif

