#include <fmt/format.h>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

struct Color : ImVec4 {
    using ImVec4::ImVec4;
};
struct Size : ImVec2 {
    using ImVec2::ImVec2;
};

void to_json(nlohmann::json& j, Color const& c) {
    j = nlohmann::json{
      {         "red", c.x},
      {       "green", c.y},
      {        "blue", c.z},
      {"transparency", c.w}
    };
}

void from_json(nlohmann::json const& j, Color& c) {
    j.at("red").get_to(c.x);
    j.at("green").get_to(c.y);
    j.at("blue").get_to(c.z);
    j.at("transparency").get_to(c.w);
}
void to_json(nlohmann::json& j, Size const& s) {
    j = nlohmann::json{
      {"sizex", s.x},
      {"sizey", s.y},
    };
}

void from_json(nlohmann::json const& j, Size& s) {
    j.at("sizex").get_to(s.x);
    j.at("sizey").get_to(s.y);
}

nlohmann::json load_json_file(std::string const& path) {
    std::ifstream  json_file(path);
    nlohmann::json return_json;
    json_file >> return_json;
    return return_json;
}

void write_json_file(std::string const& path, nlohmann::json const& json_data) {
    std::ofstream json_file(path);
    json_file << json_data.dump(4);
    json_file.close();
}

template<typename T>
T load_json(nlohmann::json const& configjson, std::string const& key) {
    if(configjson.contains(key)) {
        return configjson[key].get<T>();
    } else {
        fmt::print("key {} not found\n\r", key);
        if constexpr(std::is_same_v<T, std::string>) {
            return T{key};
        } else {
            return T{};
        }
    }
}

template<typename T, typename... Args>
T load_json(nlohmann::json const& config, std::string const& key, Args... keylist) {
    nlohmann::json unpackme = config[key];
    T              returnme = load_json<T>(unpackme, keylist...);

    return returnme;
}