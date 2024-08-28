#ifndef JSONHANDLER_HPP
#define JSONHANDLER_HPP

#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"

nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::tuple<double, double, std::chrono::time_point<std::chrono::high_resolution_clock>>>>& dataMap, const std::set<std::string>& filter_serials);

#endif
