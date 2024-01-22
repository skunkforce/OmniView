#pragma once
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

static void
save(std::map<Omniscope::Id, std::vector<std::pair<double, double>>> const
         &alignedData,
     fs::path const &outFile) {
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
  if (!fs::exists(path) && !path.empty())
    fs::create_directories(path);

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

  // Buffers
  static char storagePath_1[100] = "";
  static char storagePath_2[100] = "";
  static char mileage[10] = "";
  static char scantype[255] = "";

  ImGui::InputText("StoragePathInputField1", storagePath_1,
                   sizeof(storagePath_1));
  ImGui::SameLine();

  static bool b1 = false;
  static bool b2 = false;

  if (ImGui::BeginCombo("", "DevicesMenu")) {
    ImGui::Text("");
    ImGui::Checkbox("Device1", &b1);
    ImGui::Checkbox("Device2", &b2);
    ImGui::EndCombo();
  }

  static std::string inputvin;
  inputvin = getSubdirectoriesInFolder(language, "saves");

  // data to be written in .csv file(s)
  std::string full_content = inputvin;
  full_content += "\n";

  ImGui::InputText(
      load_json<std::string>(language, "input", "scantype", "label").c_str(),
      scantype, sizeof(scantype));
  ImGui::InputText(
      load_json<std::string>(language, "input", "mileage", "label").c_str(),
      mileage, sizeof(mileage));

  full_content += scantype;
  full_content += "\n";
  full_content += mileage;

  // ############################ Popup Storage Path Input Field 2
  // ##############################
  if (ImGui::BeginPopupModal("StoragePathInputField2", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    ImGui::InputTextWithHint("", "Enter Storage Path Input Field 2",
                             storagePath_2, IM_ARRAYSIZE(storagePath_2));
    if (ImGui::Button("Cancel")) {
      storagePath_2[0] = 0;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    if (ImGui::Button("Save"))
      ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }
  // ############# End popup

  // create a .csv file name
  auto fileName = [&](const std::string &name) {
    now = std::chrono::system_clock::now();
    now_time_t = std::chrono::system_clock::to_time_t(now);
    now_tm = *std::gmtime(&now_time_t);
    std::string filename{fmt::format("{}-{:%Y-%m-%dT%H-%M}.csv", name, now)};
    return filename;
  };

  // save data to the file
  auto saveStoragePath = [&](const std::string &fileContent,
                             const fs::path &outFile) {
    fs::path target_path{fs::current_path() / outFile};
    std::fstream file;
    file.open(target_path, std::ios::out | std::ios::app);

    if (!file.is_open()) {
      file.clear();
      fmt::print("Could not create {} for writing!\n", outFile.string());
      return;
    }

    fmt::print("Start saving {}\n", outFile.string());
    file << fileContent;
    file.flush();
    file.close();
    fmt::print("Finished saving\n");
  };

  auto makeDirectory = [&](const fs::path &second_folder) {
    auto temp_path = second_folder;
    temp_path = temp_path.lexically_normal();
    temp_path = temp_path.relative_path();

    auto first_folder = load_json<fs::path>(config, "scanfolder");
    fs::path complete_path = first_folder / temp_path;

    // create directory in the given path
    fs::create_directories(complete_path);
    return complete_path;
  };

  if (ImGui::Button(load_json<std::string>(language, "button", "save").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
    flagDataNotSaved = false;
    std::string filename = fileName(mileage);

    if (captureData.empty())
      ImGui::CloseCurrentPopup();
    else {
      auto complete_path = makeDirectory(fs::path(inputvin) / scantype);
      save(captureData, path / complete_path / filename);
      ImGui::CloseCurrentPopup();
    }
  }

  ImGui::SameLine();
  if (ImGui::Button(load_json<std::string>(language, "button", "back").c_str(),
                    {0, 0}))
    ImGui::CloseCurrentPopup();

  auto storagePath = [=]() {
    ImGui::SameLine();
    ImGui::Dummy({550, 0});
    ImGui::SameLine();
    if (ImGui::Button("Save Path")) {
      fs::path complete_path1, complete_path2;

      if (b1 && b2) {
        if (path[0] == '\0') {
          complete_path1 = makeDirectory(storagePath_1);
          complete_path2 = makeDirectory(storagePath_2);
        } else {
          complete_path1 = makeDirectory(fs::path(path) / storagePath_1);
          complete_path2 = makeDirectory(fs::path(path) / storagePath_2);
        }
        std::string filename1 = fileName("device1");
        std::string filename2 = fileName("device2");
        saveStoragePath(full_content, complete_path1 / filename1);
        storagePath_1[0] = 0;
        saveStoragePath(full_content, complete_path2 / filename2);
        storagePath_2[0] = 0;
      } else {
        if (path[0] == '\0')
          complete_path1 = makeDirectory(storagePath_1);
        else
          complete_path1 = makeDirectory(fs::path(path) / storagePath_1);
        std::string filename = fileName(b1 ? "device1" : "device2");
        saveStoragePath(full_content, complete_path1 / filename);
        storagePath_1[0] = 0;
      }
    }
  };

  // if only one of the devices is selected
  if ((b1 || b2) && !(b1 && b2))
    storagePath();

  // if both devices are selected
  else if (b1 && b2) {
    storagePath();
    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
    if (ImGui::Button(" + "))
      ImGui::OpenPopup("StoragePathInputField2");

    ImGui::PopStyleVar();
    ImGui::Dummy({850, 0});
    ImGui::SameLine();
    ImGui::Text("Add another path");
  }
}
