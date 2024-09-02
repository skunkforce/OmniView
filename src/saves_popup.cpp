#include <fstream>
#include <future>
#include <charconv>
#include "popups.hpp"
#include "look_up_saves.hpp"
#include "imgui_stdlib.h"
#include "languages.hpp"
#include "style.hpp"

static void save(const Omniscope::Id &device,
                 const std::vector<std::pair<double, double>> &values,
                 const fs::path &outFile, std::string allData,
                 std::atomic_uint64_t &y_indx, std::string filename,
                 std::atomic_uint16_t &saved_files_cnt) {
  std::string serialFilename = device.serial + " " + filename;
  allData += fmt::format(",{},{},{}\n", device.type, serialFilename,
                         device.sampleRate);
  std::string fileContent;
  fileContent.resize_and_overwrite(
      // ten bytes for each y_value, nine for the number
      // and one new line as a separator between the numbers
      values.size() * 10, // take const ref to values
      [&values = std::as_const(values), &y_indx](char *begin, std::size_t) {
        auto end = begin;
        constexpr unsigned factor{100'000};
        for (; y_indx < values.size(); y_indx++) {
          double value = values[y_indx].second * factor;
          value = std::floor(value);
          value /= factor;
          end = std::to_chars(end, end + 9, value).ptr;
          *end++ = '\n';
        }
        return end - begin;
      });
  // create a .csv file to write to it
  std::ofstream file(outFile, std::ios::app);
  if (!file.is_open()) {
    file.clear();
    fmt::println("Could not create {} for writing!", outFile.string());
    return;
  }
  fmt::println("Start saving {}.", outFile.string());
  file << allData << fileContent;
  file.flush();
  file.close();
  fmt::println("Finished saving.");
  y_indx = 0; // reset index after each save
  ++saved_files_cnt;
}
void saves_popup(nlohmann::json const &config, nlohmann::json const &language,
                 std::chrono::system_clock::time_point &now,
                 std::time_t &now_time_t, std::tm &now_tm,
                 bool &flagDataNotSaved, decltype(captureData) &liveDvcs) {
  ImGui::SetItemDefaultFocus();
  const size_t devicesSz{liveDvcs.size()};
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
  ImGui::Text(appLanguage[Key::Select_Storage_Location]);
  ImGui::NewLine();

  ImGui::InputTextWithHint("##Lable1", "\".../OmniView/saves/\"",
                           &inptTxtFields[0]);
  ImGui::SameLine();
  static ImGui::FileBrowser directoryBrowser(
      ImGuiFileBrowserFlags_SelectDirectory);
  if (ImGui::Button(appLanguage[Key::Browse]))
    directoryBrowser.Open();

  directoryBrowser.Display();

  if (directoryBrowser.HasSelected()) {
    hasSelectedPathArr[0].b = true;
    selectedPathArr[0] = directoryBrowser.GetPwd().string();
    inptTxtFields[0] = selectedPathArr[0];
    directoryBrowser.ClearSelected();
  }

  ImGui::Separator();
  SetHorizontalSepeareatorColours();
  ImGui::NewLine();
  ImGui::Text(appLanguage[Key::Select_Devices]);
  ImGui::NewLine();

  if (ImGui::BeginCombo("##Combo", appLanguage[Key::Devices_Menu])) {
    for (size_t i = 0; i < devicesSz; i++)
      ImGui::Checkbox(fmt::format("Device {}", i + 1).c_str(),
                      &(dvcCheckedArr[i].b));
    ImGui::EndCombo();
  }

  static char scantype[255];
  static char vin[19];
  static char mileage[10];
  std::string inputvin =
      getSubdirectoriesInFolder(language, "saves", scantype, vin, mileage);

  // data to be written in .csv file(s)
  std::string allData = scantype;
  allData += ",";
  allData += inputvin;
  allData += ",";
  allData += mileage;

  auto count_checked_devices = [&]() {
    unsigned deviceCnt{0};
    for (size_t i = 0; i < devicesSz; ++i)
      if (dvcCheckedArr[i].b)
        deviceCnt++;
    return deviceCnt;
  };

  SetHorizontalSepeareatorColours();

  // ############################ Popup Storage Path Input Field(s)
  // ##############################

  if (ImGui::BeginPopupModal("StoragePathInputFields", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetItemDefaultFocus();
    for (unsigned i = 1; i < count_checked_devices(); ++i) {
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

    if (ImGui::Button(appLanguage[Key::Back])) {
      for (size_t i = 1; i < devicesSz; ++i) {
        hasSelectedPathArr[i].b = false;
        selectedPathArr[i].clear();
        inptTxtFields[i].clear();
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    if (ImGui::Button(appLanguage[Key::OK]))
      ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }
  // ############# End popup

  // make a .csv file name
  auto mkFileName = [&](const std::string &name) {
    now = std::chrono::system_clock::now();
    now_time_t = std::chrono::system_clock::to_time_t(now);
    now_tm = *std::gmtime(&now_time_t);
    std::string filename{fmt::format("{}-{:%Y-%m-%dT%H-%M}.csv", name, now)};
    return filename;
  };

  // make a directory (if not exists) and return a path to it
  auto mkdir = [&](bool hasSelectedPath, std::string selectedPath,
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
    } else
      complete_path = dest / first_folder;

    if (!fs::exists(complete_path))
      fs::create_directories(complete_path);

    return complete_path / outFile;
  };

  if (const unsigned checkedDvc_cnt = count_checked_devices())
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

  ImGui::Separator();
  ImGui::NewLine();
  if (ImGui::Button(appLanguage[Key::Back]))
    ImGui::CloseCurrentPopup();
  ImGui::SameLine(ImGui::GetWindowWidth() * 0.75f); // offset from start x

  static std::future<void> future;
  static std::atomic_uint64_t y_indx, valuesSize;
  static std::atomic_uint16_t saved_files_cnt;
  static unsigned checked_devices_cnt;
  static bool progress;

  if (ImGui::Button(appLanguage[Key::Save])) {
    checked_devices_cnt = count_checked_devices();
    flagDataNotSaved = false;
    future = std::async( // const reference to the container
        std::launch::async, [=, &liveDvcs = std::as_const(liveDvcs)] {
          for (size_t i{}; const auto &[device, values] : liveDvcs) {
            // measurement saving preparation if device is checked
            if (dvcCheckedArr[i].b) {
              fs::path path;
              auto filename = mkFileName(fmt::format("device{}", i + 1));
              if (hasSelectedPathArr[i].b) {
                path = mkdir(true, selectedPathArr[i], "", filename);
                hasSelectedPathArr[i].b = false;
              } else if (!inptTxtFields[i].empty()) {
                path = mkdir(false, "", inptTxtFields[i], filename);
                inptTxtFields[i].clear();
              } else
                path = mkdir(false, "", "", filename);
              valuesSize = values.size();
              // save measurement of each device in a separate file
              save(device, values, path, allData, y_indx, filename,
                   saved_files_cnt);
            }
            i++;
          }
        });
    progress = true;
  }
  if (progress) { // check selected devices are saved
    if (saved_files_cnt == checked_devices_cnt) {
      future.get();
      progress = false;
      inptTxtFields.clear(); // reset storage location(s)
      dvcCheckedArr.clear(); // rest check boxes
      saved_files_cnt = 0;
      ImGui::CloseCurrentPopup();
    } else {
      ImGui::ProgressBar((float)y_indx / valuesSize, {0.f, 0.f});
      ImGui::SameLine();
      ImGui::Text(appLanguage[Key::Saving]);
    }
  }
}