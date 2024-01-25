#pragma once
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace fs = std::filesystem;

static void
save(std::map<Omniscope::Id, std::vector<std::pair<double, double>>> const
         &alignedData,
     fs::path const &outFile, std::string allData) {

  // save(captureData, complete_path, allData);

  // fmt::println("\nInside save():");
  // fmt::println("outFile is: \n{} and allData is: \n{}\n", outFile, allData);
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

  // create a .csv file to write to it
  std::fstream file{outFile};
  file.open(outFile, std::ios::out | std::ios::app);

  if (!file.is_open()) {
    file.clear();
    fmt::print("Could not create {} for writing!\n", outFile.string());
    return;
  }

  fmt::print("Start saving {}\n", outFile.string());
  file << allData << '\n' << fileContent;
  file.flush();
  file.close();
  fmt::print("finished save\n");
}

void saves_popup(nlohmann::json const &config, nlohmann::json const &language,
                 std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
                     &captureData,
                 std::chrono::system_clock::time_point &now,
                 std::time_t &now_time_t, std::tm &now_tm, std::string &path,
                 bool &flagDataNotSaved,
                 std::vector<std::shared_ptr<OmniscopeDevice>> devices) {

  ImGui::SetItemDefaultFocus();

  // Have address of bool for std::vector
  struct BoolWrapper {
    BoolWrapper(bool _b) : b(_b) {}
    bool b;
  };

  // connected devices at runt-time
  const static size_t devicesSz{devices.size()};
  constexpr size_t inptTxtArrSz{100};

  // buffer array for the input text field(s)
  static std::vector<std::vector<char>> inptTxtFilds(
      devicesSz, std::vector<char>(inptTxtArrSz, 0));

  ImGui::InputTextWithHint("##HintLable", "\"Desktop/OmniView/saves/\"",
                           inptTxtFilds[0].data(), inptTxtArrSz);

  static std::vector<BoolWrapper> hasSelectedPathArr(devicesSz, false);
  static std::vector<std::string> selectedPathArr(devicesSz, "");

  // select directory instead of regular file
  static ImGui::FileBrowser fileBrowser(ImGuiFileBrowserFlags_SelectDirectory);

  ImGui::SameLine();
  if (ImGui::Button("Durchsuchen"))
    fileBrowser.Open();

  fileBrowser.Display();

  if (fileBrowser.HasSelected()) {
    hasSelectedPathArr[0].b = true;
    selectedPathArr[0] = fileBrowser.GetPwd().string();
    fileBrowser.ClearSelected();
  }

  static std::vector<BoolWrapper> dvcChackedArr(devicesSz, false);
  std::stringstream ss;

  ImGui::SameLine();
  if (ImGui::BeginCombo("##", "DevicesMenu")) {
    ImGui::Text("");
    for (size_t i = 0; i < devicesSz; i++) {
      ss << "Device " << i + 1;
      ImGui::Checkbox(ss.str().c_str(), &((dvcChackedArr.begin() + i)->b));
      ss.str(std::string());
    }
    ImGui::EndCombo();
  }

  static char scantype[255] = "";
  static char vin[18] = "";
  static char mileage[10] = "";
  static std::string inputvin;
  inputvin =
      getSubdirectoriesInFolder(language, "saves", scantype, vin, mileage);

  // data to be written in .csv file(s)
  std::string allData = scantype;
  allData += ", ";
  allData += inputvin;
  allData += ", ";
  allData += mileage;
  allData += "\n";

  static std::vector<size_t> indxArr;

  auto selectedDevicesCnt = [&]() {
    size_t deviceCnt{0};
    for (size_t i = 0; i < devicesSz; ++i)
      if (dvcChackedArr[i].b) {
        deviceCnt++;
        indxArr.emplace_back(i);
      }
    return deviceCnt;
  };

  // ############################ Popup Storage Path Input Field(s)
  // ##############################

  if (ImGui::BeginPopupModal("StoragePathInputFields", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    for (size_t i = 1; i < selectedDevicesCnt(); ++i) {
      // each iteration to get a unique ID
      ImGui::PushID(i);
      ImGui::InputTextWithHint("##HintLable", "\"Desktop/OmniView/saves/\"",
                               inptTxtFilds[i].data(), inptTxtArrSz);
      ImGui::SameLine();
      if (ImGui::Button("Durchsuchen"))
        fileBrowser.Open();

      fileBrowser.Display();

      if (fileBrowser.HasSelected()) {
        hasSelectedPathArr[i].b = true;
        selectedPathArr[i] = fileBrowser.GetPwd().string();
        fileBrowser.ClearSelected();
      }
      ImGui::PopID();
    }

    indxArr.clear();

    if (ImGui::Button("Cancel")) {
      for (size_t i = 1; i < devicesSz; ++i) {
        hasSelectedPathArr[i].b = false;
        selectedPathArr[i].clear();
        inptTxtFilds[i].clear();
      }

      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    if (ImGui::Button("Save"))
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

  if (const size_t cnt = selectedDevicesCnt()) {
    if (cnt > 1) {
      ImGui::SameLine();
      ImGui::Dummy({565, 0});
      ImGui::SameLine();
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 50);
      if (ImGui::Button(" + "))
        ImGui::OpenPopup("StoragePathInputFields");

      ImGui::PopStyleVar();
      ImGui::SetItemTooltip("Add another path");

      ImGui::Dummy({750, 0});
      ImGui::SameLine();
    }

    // to save captureData from main.cpp into file
    if (ImGui::Button(
            load_json<std::string>(language, "button", "save").c_str(),
            ImVec2(load_json<Size>(config, "button")))) {
      flagDataNotSaved = false;

      if (captureData.empty()) {
        fmt::println("captureData is empty");
        ImGui::CloseCurrentPopup();
      } else {
        fs::path complete_path;
        for (size_t i = 0; i < cnt; ++i) {
          ss.str(std::string());
          ss << "device" << indxArr[i] + 1;
          auto filename = makeFileName(ss.str());

          if (hasSelectedPathArr[i].b) {
            complete_path =
                makeDirectory(true, selectedPathArr[i], "", filename);
            save(captureData, complete_path, allData);
            hasSelectedPathArr[i].b = false;
          } else if (!inptTxtFilds[i].empty()) {
            complete_path =
                makeDirectory(false, "", inptTxtFilds[i].data(), filename);
            save(captureData, complete_path, allData);
            inptTxtFilds[i].clear();
          } else {
            complete_path = makeDirectory(false, "", "", filename);
            save(captureData, complete_path, allData);
          }
        }
        indxArr.clear();
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::SameLine();
  }

  if (ImGui::Button(load_json<std::string>(language, "button", "back").c_str(),
                    {0, 0}))
    ImGui::CloseCurrentPopup();
}