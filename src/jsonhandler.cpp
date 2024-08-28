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
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::get<2>(tuple).time_since_epoch()).count();
            nlohmann::json dataPoint = {
                {"Id", id.serial},
                {"x", std::get<0>(tuple)},
                {"y", std::get<1>(tuple)},
                {"timestamp", timestamp}
            };
            jsonData.push_back(dataPoint);
        }
    }
    return jsonData;
}
