#ifndef HANDLER_HEADER_HPP
#define HANDLER_HEADER_HPP

#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <atomic>

// global variables
inline OmniscopeDeviceManager deviceManager{};
inline std::vector<std::shared_ptr<OmniscopeDevice>> devices;
inline std::map<Omniscope::Id, std::array<float, 3>> colorMap;
inline std::optional<OmniscopeSampler> sampler{};
inline std::map<Omniscope::Id, std::vector<std::pair<double, double>>> captureData;
inline std::atomic<bool> running{true};

void initDevices();
void signalHandler(int signal);

#endif
