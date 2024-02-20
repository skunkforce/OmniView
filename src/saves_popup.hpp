#pragma once
#include "popups.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

static void save(const Omniscope::Id &device,
                 const std::vector<std::pair<double, double>> &values,
                 fs::path const &outFile, std::string allData) {

  std::string fileContent = fmt::format("{}-{}\n", device.type, device.serial);
  for (std::size_t i{}; i < values.size(); ++i) {
    fileContent += fmt::format("{},", i);
    if (values[i].second)
      fileContent += fmt::format("{}\n", values[i].second);
  }

  // create a .csv file to write to it
  std::fstream file{outFile};
  file.open(outFile, std::ios::out | std::ios::app);

  if (!file.is_open()) {
    file.clear();
    fmt::print("Could not create {} for writing!\n", outFile.string());
    return;
  }

  fmt::print("Start saving {}.\n", outFile.string());
  file << allData << fileContent;
  file.flush();
  file.close();
  fmt::print("Finished saving.\n");
}

void saves_popup(nlohmann::json const &config, nlohmann::json const &language,
                 std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
                     &captureData,
                 std::chrono::system_clock::time_point &now,
                 std::time_t &now_time_t, std::tm &now_tm,
                 bool &flagDataNotSaved,
                 const std::optional<OmniscopeSampler> &sampler) {

  ImGui::SetItemDefaultFocus();

  const size_t devicesSz{sampler->sampleDevices.size()};
  // input text fields
  static std::vector<std::string> inptTxtFields(devicesSz);
  inptTxtFields.resize(devicesSz);
  // number of selected paths
  static std::vector<BoolWrapper> hasSelectedPathArr(devicesSz, false);
  hasSelectedPathArr.resize(devicesSz);
  // selected paths
  static std::vector<std::string> selectedPathArr(devicesSz);
  selectedPathArr.resize(devicesSz);
  // devices checkboxes
  static std::vector<BoolWrapper> dvcCheckedArr(devicesSz, false);
  dvcCheckedArr.resize(devicesSz);

  ImGui::InputTextWithHint("##Lable1", "\".../OmniView/saves/\"",
                           &inptTxtFields[0]);
  ImGui::SameLine();
  if (ImGui::Button("Browse"))
    directoryBrowser.Open();

  directoryBrowser.Display();

  if (directoryBrowser.HasSelected()) {
    hasSelectedPathArr[0].b = true;
    selectedPathArr[0] = directoryBrowser.GetPwd().string();
    inptTxtFields[0] = selectedPathArr[0];
    directoryBrowser.ClearSelected();
  }

  ImGui::SameLine();
  if (ImGui::BeginCombo("##Combo", "DevicesMenu")) {
    std::stringstream ss;
    for (size_t i = 0; i < devicesSz; i++) {
      ss << "Device " << i + 1;
      ImGui::Checkbox(ss.str().c_str(), &(dvcCheckedArr[i].b));
      ss.str(std::string());
    }
    ImGui::EndCombo();
  }

  static char scantype[255] = "";
  static char vin[18] = "";
  static char mileage[10] = "";
  std::string inputvin =
      getSubdirectoriesInFolder(language, "saves", scantype, vin, mileage);

  // data to be written in .csv file(s)
  std::string allData = scantype;
  allData += ",";
  allData += inputvin;
  allData += ",";
  allData += mileage;
  allData += "\n";

  auto count_checked_devices = [&]() {
    size_t deviceCnt{0};
    for (size_t i = 0; i < devicesSz; ++i)
      if (dvcCheckedArr[i].b)
        deviceCnt++;
    return deviceCnt;
  };

  // ############################ Popup Storage Path Input Field(s)
  // ##############################

  if (ImGui::BeginPopupModal("StoragePathInputFields", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    for (size_t i = 1; i < count_checked_devices(); ++i) {
      // each iteration to get a unique ID
      ImGui::PushID(i);
      ImGui::InputTextWithHint("##Lable2", "\".../OmniView/saves/\"",
                               &inptTxtFields[i]);
      ImGui::SameLine();
      if (ImGui::Button("Browse"))
        directoryBrowser.Open();

      directoryBrowser.Display();

      if (directoryBrowser.HasSelected()) {
        hasSelectedPathArr[i].b = true;
        selectedPathArr[i] = directoryBrowser.GetPwd().string();
        inptTxtFields[i] = selectedPathArr[i];
        directoryBrowser.ClearSelected();
      }
      ImGui::PopID();
    }

    if (ImGui::Button("Cancel")) {
      for (size_t i = 1; i < devicesSz; ++i) {
        hasSelectedPathArr[i].b = false;
        selectedPathArr[i].clear();
        inptTxtFields[i].clear();
      }

      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    if (ImGui::Button("Close"))
      ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }
  // ############# End popup

  // create a .csv file name
  auto makeFileName = [&](const std::string &name) {
    now = std::chrono::system_clock::now();
    now_time_t = std::chrono::system_clock::to_time_t(now);
    now_tm = *std::gmtime(&now_time_t);
    std::string filename{fmt::format("{}-{:%Y-%m-%dT%H-%M}.csv", name, now)};
    return filename;
  };

  // create a director (if not exists) and return a path to it
  auto makeDirectory = [&](bool hasSelectedPath, std::string selectedPath,
                           std::string second_folder, std::string outFile) {
    fs::path complete_path;
    auto first_folder = load_json<fs::path>(config, "scanfolder");

    // get to ../Omniview
    std::string source = fs::current_path().string();
    std::string omni{"OmniView"}, dest{""};
    auto it = source.find(omni);

    if (it != std::string::npos) {
      const size_t sz{it + std::size(omni)};
      for (size_t i = 0; i < sz; ++i)
        dest += source[i];
    }

    if (hasSelectedPath)
      complete_path = selectedPath;

    else if (!second_folder.empty()) {
      fs::path tempPath = second_folder;
      tempPath = tempPath.lexically_normal();
      tempPath = tempPath.relative_path();
      complete_path = dest / first_folder / tempPath;
    }

    else
      complete_path = dest / first_folder;

    if (!fs::exists(complete_path))
      fs::create_directories(complete_path);

    return complete_path / outFile;
  };

  if (const size_t checkedDvc_cnt = count_checked_devices()) {
    if (checkedDvc_cnt > 1) {
      ImGui::SameLine();
      ImGui::Dummy({200, 0});
      ImGui::SameLine();
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 50);
      if (ImGui::Button(" + "))
        ImGui::OpenPopup("StoragePathInputFields");

      ImGui::PopStyleVar();
      ImGui::SetItemTooltip("Add another path");
    }

    // to save captureData from main.cpp into file
    if (ImGui::Button(" save ")) {
      flagDataNotSaved = false;

      if (captureData.empty()) {
        fmt::println("captureData is empty");
        ImGui::CloseCurrentPopup();
      }
      fs::path complete_path;

      size_t i{0};
      for (const auto &[device, values] : captureData) {
        if (dvcCheckedArr[i].b) {
          std::stringstream ss;
          ss << "device" << i + 1;
          auto filename = makeFileName(ss.str());
          if (hasSelectedPathArr[i].b) {
            complete_path =
                makeDirectory(true, selectedPathArr[i], "", filename);
            save(device, values, complete_path, allData);
            hasSelectedPathArr[i].b = false;
          } else if (!inptTxtFields[i].empty()) {
            complete_path =
                makeDirectory(false, "", inptTxtFields[i], filename);
            save(device, values, complete_path, allData);
            inptTxtFields[i].clear();
          } else {
            complete_path = makeDirectory(false, "", "", filename);
            save(device, values, complete_path, allData);
          }
        }
        ++i;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
  }

  if (ImGui::Button(" back "))
    ImGui::CloseCurrentPopup();
}