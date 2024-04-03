// handler.hpp
#ifndef HANDLER_HEADER_HPP
#define HANDLER_HEADER_HPP

#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>

std::vector<std::string> getAvailableLanguages(const std::string &);
void addPlots(
    const char *,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &,
    const bool, std::function<void(double)>,
    std::map<Omniscope::Id, std::array<float, 3>> &);

void initDevices(OmniscopeDeviceManager &,
                 std::vector<std::shared_ptr<OmniscopeDevice>> &,
                 std::map<Omniscope::Id, std::array<float, 3>> &);

void devicesList(std::map<Omniscope::Id, std::array<float, 3>> &,
                 std::optional<OmniscopeSampler> &,
                 std::vector<std::shared_ptr<OmniscopeDevice>> &);
void set_config(const std::string &);

void set_json(nlohmann::json &);

void rstSettings(
    std::optional<OmniscopeSampler> &,
    std::vector<std::shared_ptr<OmniscopeDevice>> &, std::set<std::string> &,
    OmniscopeDeviceManager &,
    std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

#endif
