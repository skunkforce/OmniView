// handler.hpp
#ifndef HANDLER_HEADER_HPP
#define HANDLER_HEADER_HPP

#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "languages.hpp"
#include <implot.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>

// global variables
inline OmniscopeDeviceManager deviceManager{};
inline std::vector<std::shared_ptr<OmniscopeDevice>> devices;
inline std::map<Omniscope::Id, std::array<float, 3>> colorMap;
inline std::set<std::string> savedFileNames; // unique and ordered filenames
inline std::optional<OmniscopeSampler> sampler{};
inline std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
    captureData;
void addPlots(
    const char *,
    std::function<void(double, std::string, ImAxis_, double, double)>);
void initDevices();
void devicesList(bool const &flagPaused);
void load_files(decltype(captureData) &, std::map<Omniscope::Id, std::string> &,
                bool &);
void set_config(std::string_view);
void set_json(const nlohmann::json &);
void set_inital_config(nlohmann::json &);
void rstSettings(const decltype(captureData) &);

#endif