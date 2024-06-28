// handler.hpp
#ifndef HANDLER_HEADER_HPP
#define HANDLER_HEADER_HPP

#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "languages.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <implot.h>

struct AxisInfo {
    std::pair<Omniscope::Id, std::vector<std::pair<double, double>>&> data;
    std::pair<std::string, ImAxis_> egu;
    std::string timebase;

    AxisInfo(
        std::pair<Omniscope::Id, std::vector<std::pair<double, double>>&> data_,
        std::pair<std::string, ImAxis_> egu_, std::string timebase_)
        : data{data_}, egu{egu_}, timebase{timebase_} {}
};

// global variables
inline OmniscopeDeviceManager deviceManager{};
inline std::vector<std::shared_ptr<OmniscopeDevice>> devices;
inline std::map<Omniscope::Id, std::array<float, 3>> colorMap;
inline std::set<std::string> savedFileNames; // unique and ordered filenames
inline std::optional<OmniscopeSampler> sampler{};
inline std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
    captureData;

inline std::vector<AxisInfo> plotAxes;
void addPlots(const char *, const bool, std::function<void(double, std::string, ImAxis_, double, double)>);
void parseDeviceMetaData(Omniscope::MetaData, std::shared_ptr<OmniscopeDevice>&);
void initDevices();
void devicesList(bool const& flagPaused);
void load_files(decltype(captureData) &, std::map<Omniscope::Id, std::string> &,
                bool &);
void set_config(const std::string &);
void set_json(nlohmann::json &);
void set_inital_config(nlohmann::json &);
void rstSettings(const decltype(captureData) &);

#endif
