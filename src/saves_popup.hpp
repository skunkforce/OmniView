#pragma once
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include <nlohmann/json.hpp>

static void
save(std::map<Omniscope::Id, std::vector<std::pair<double, double>>> const
         &alignedData,
     std::filesystem::path const &outFile) {
  auto minSize = std::numeric_limits<std::size_t>::max();

  std::vector<std::vector<std::pair<double, double>> const *> data;

  std::string fileContent;

  for (auto sep = std::string_view{};
       auto const &[device, values] : alignedData) {
    fileContent += sep;
    fileContent += "\"";
    fileContent += fmt::format("{}-{}", device.type, device.serial);
    fileContent += "\"";
    sep = ",";
    minSize = std::min(values.size(), minSize);
    data.push_back(std::addressof(values));
  }
  fileContent += '\n';

  for (std::size_t i{}; i < minSize; ++i) {
    fileContent += fmt::format("{}", i);
    fileContent += ",";
    for (auto sep = std::string_view{}; auto const &vec : data) {
      auto const dataPair = (*vec)[i];
      fileContent += sep;
      if (dataPair.second) {
        fileContent += fmt::format("{}", dataPair.second);
      }
      sep = ",";
    }
    fileContent += '\n';
  }

  fileContent += '\n';

  auto path = outFile;
  path.remove_filename();
  if (!std::filesystem::exists(path) && !path.empty()) {
    std::filesystem::create_directories(path);
  }

  fmt::print("start save {}\n", outFile.string());
  std::ofstream file{outFile};
  // fmt::print("{}\n", fileContent);
  file << fileContent;
  file.flush();
  file.close();

  fmt::print("finished save\n");
}

void saves_popup(nlohmann::json const &config, nlohmann::json const &language,
                 std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
                     &captureData,
                 std::chrono::system_clock::time_point &now,
                 std::time_t &now_time_t, std::tm &now_tm, std::string &path,
                 bool &flagDataNotSaved) {
  ImGui::SetItemDefaultFocus();

  static std::string inputvin;

  static char mileage[10] = "";
  static char scantype[255] = "";

  inputvin = getSubdirectoriesInFolder(language, "saves");
  ImGui::InputText(
      load_json<std::string>(language, "input", "scantype", "label").c_str(),
      scantype, sizeof(scantype));
  ImGui::InputText(
      load_json<std::string>(language, "input", "mileage", "label").c_str(),
      mileage, sizeof(mileage));

  if (ImGui::Button(load_json<std::string>(language, "button", "save").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
    flagDataNotSaved = false;
    now = std::chrono::system_clock::now();

    now_time_t = std::chrono::system_clock::to_time_t(now);
    now_tm = *std::gmtime(&now_time_t);

    std::string_view path_sv{path.data()};
    std::string filename{fmt::format("{}-{:%Y-%m-%dT%H-%M}.csv", mileage, now)};
    std::filesystem::path path_path = path_sv;
    if (captureData.empty()) {
      ImGui::CloseCurrentPopup();
    } else {
      // create the given folder_structure
      std::filesystem::path first_folder =
          load_json<std::filesystem::path>(config, "scanfolder");
      std::filesystem::path complete_path = first_folder / inputvin / scantype;
      std::filesystem::create_directories(complete_path);

      save(captureData, path_path / complete_path / filename);

      ImGui::CloseCurrentPopup();
    }
  }
}
