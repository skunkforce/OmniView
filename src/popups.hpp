// popups.hpp

#ifndef POPUPS_HEADER_HPP
#define POPUPS_HEADER_HPP

#include <optional>
#include <set>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
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
std::string generateTrainingData(
    bool &,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &,
    std::set<std::string> &, const nlohmann::json &);
void info_popup(std::string_view, std::string_view);
void saves_popup(
    nlohmann::json const &, nlohmann::json const &,
    std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &,
    std::chrono::system_clock::time_point &, std::time_t &now_time_t,
    std::tm &now_tm, bool &);

#endif
