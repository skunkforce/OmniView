#include "jsonhandler.hpp"

nlohmann::json captureDataToJson(const std::map<Omniscope::Id, std::vector<std::tuple<double, double, std::chrono::time_point<std::chrono::high_resolution_clock>>>>& dataMap, const std::set<std::string>& filter_serials) {
    nlohmann::json jsonData = nlohmann::json::array();

    for (const auto& entry : dataMap) {
        const Omniscope::Id& id = entry.first;
        if (!filter_serials.empty() && filter_serials.find(id.serial) == filter_serials.end()) {
            continue;
        }
        const auto& value = entry.second;
        for (const auto& tuple : value) {
            int64_t timestamp = std::get<2>(tuple).time_since_epoch().count();

            nlohmann::json dataPoint = {

                // DEBUG ID
                // {"Id", id.serial},
                // Replaced by a timestamp
                // {"x", std::get<0>(tuple)},

                {"value", std::get<1>(tuple)},
                {"timestamp", timestamp}
            };
            jsonData.push_back(dataPoint);
        }
    }
    return jsonData;
}
