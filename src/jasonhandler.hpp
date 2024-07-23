#pragma once

#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

static nlohmann::json load_json_file(std::string const &path) {
  std::ifstream json_file(path);
  nlohmann::json return_json;
  json_file >> return_json;
  return return_json;
}

static void write_json_file(std::string const &path,
                            nlohmann::json const &json_data) {
  std::ofstream json_file(path);
  json_file << json_data.dump(4);
  json_file.close();
}

template <typename T>
T load_json(nlohmann::json const &configjson, std::string const &key) {
  if (configjson.contains(key)) {
    return configjson[key].get<T>();
  } else {
    fmt::print("key {} not found\n\r", key);
    if constexpr (std::is_same_v<T, std::string>) {
      return T{key};
    } else {
      return T{};
    }
  }
}

template <typename T, typename... Args>
T load_json(nlohmann::json const &config, std::string const &key,
            Args... keylist) {
  nlohmann::json unpackme = config[key];
  T returnme = load_json<T>(unpackme, keylist...);

  return returnme;
}

