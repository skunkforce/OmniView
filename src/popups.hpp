// popups.hpp
#ifndef POPUPS_HEADER_HPP
#define POPUPS_HEADER_HPP

#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>
#include <set>
#include "imgui.h"
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"

// Have address of bool for std::vector
struct BoolWrapper {
  BoolWrapper() : b(false) {}
  BoolWrapper(bool _b) : b(_b) {}
  bool b;
};

inline ImGui::FileBrowser fileBrowser;
inline ImGui::FileBrowser
    directoryBrowser(ImGuiFileBrowserFlags_SelectDirectory);
void generateTrainingData(
    bool &,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &,
    std::set<std::string> &);
void info_popup(std::string_view, std::string_view);
void saves_popup(
    nlohmann::json const &, nlohmann::json const &,
    std::chrono::system_clock::time_point &, std::time_t &now_time_t,
    std::tm &now_tm, bool &);

#endif