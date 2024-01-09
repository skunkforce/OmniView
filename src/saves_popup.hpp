#pragma once
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

namespace fs = std::filesystem;

static void save(std::map<Omniscope::Id, std::vector<std::pair<double, double>>> const
                &alignedData, fs::path const &outFile)
{
  auto minSize = std::numeric_limits<std::size_t>::max();
  std::vector<std::vector<std::pair<double, double>> const *> data;
  std::string fileContent;

  for (auto sep = std::string_view{};
       auto const &[device, values] : alignedData)
  {
    fileContent += sep;
    fileContent += "\"";
    fileContent += fmt::format("{}-{}", device.type, device.serial);
    fileContent += "\"";
    sep = ",";
    minSize = std::min(values.size(), minSize);
    data.push_back(std::addressof(values));
  }
  fileContent += '\n';

  for (std::size_t i{}; i < minSize; ++i)
  {
    fileContent += fmt::format("{}", i);
    fileContent += ",";
    for (auto sep = std::string_view{}; auto const &vec : data)
    {
      auto const dataPair = (*vec)[i];
      fileContent += sep;
      if (dataPair.second)
      {
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
                 &captureData, std::chrono::system_clock::time_point &now,
                 std::time_t &now_time_t, std::tm &now_tm, std::string &path,
                 bool &flagDataNotSaved)
{
  ImGui::SetItemDefaultFocus();

  // Buffers
  static char storagePath_1[100] = "";
  static char storagePath_2[100] = "";
  ImGui::InputTextWithHint("##myTextHint1", "\"Desktop/OmniView/saves/\"",
                           storagePath_1, IM_ARRAYSIZE(storagePath_1));
  ImGui::SameLine();

  static bool b1 = false;
  static bool b2 = false;
  static bool hasSelectedPath = false;

 // select directory instead of regular file
  static ImGui::FileBrowser fileBrowser(ImGuiFileBrowserFlags_SelectDirectory);
  if (ImGui::Button("Duechsuchen"))
    fileBrowser.Open();

  fileBrowser.Display();
  static std::string selectedPath = "";

  if (fileBrowser.HasSelected())
  {
    hasSelectedPath = true;
    selectedPath = fileBrowser.GetPwd();
    fileBrowser.ClearSelected();
  }

  ImGui::SameLine();

  if (ImGui::BeginCombo("", "DevicesMenu"))
  {
    ImGui::Text("");
    ImGui::Checkbox("Device1", &b1);
    ImGui::Checkbox("Device2", &b2);
    ImGui::EndCombo();
  }

  static char scantype[255] = "";
  static char vin[18] = "";
  static char mileage[10] = "";
  static std::string inputvin;
  inputvin = getSubdirectoriesInFolder(language, "saves", scantype, vin, mileage);

  // data to be written in .csv file(s)
  std::string allData = scantype;
  allData += "\n";
  allData += inputvin;
  allData += "\n";
  allData += mileage;
  allData += "\n";

  // ############################ Popup Storage Path Input Field 2
  // ##############################
  if (ImGui::BeginPopupModal("StoragePathInputField2", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::SetItemDefaultFocus();
    ImGui::InputTextWithHint("##myTextHint2", "Enter Storage Path Input Field 2",
                             storagePath_2, IM_ARRAYSIZE(storagePath_2));
    if (ImGui::Button("Cancel"))
    {
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
  auto fileName = [&](const std::string &name)
  {
    now = std::chrono::system_clock::now();
    now_time_t = std::chrono::system_clock::to_time_t(now);
    now_tm = *std::gmtime(&now_time_t);
    std::string filename{fmt::format("{}-{:%Y-%m-%dT%H-%M}.csv", name, now)};
    return filename;
  };

  // save data to the file
  auto saveData = [&](const std::string &fileContent, const fs::path &outFile)
  {
    fs::path target_path;
    if (hasSelectedPath)
    {
      target_path = outFile;
      selectedPath.clear();
      hasSelectedPath = false;
    }
    else
      target_path = fs::current_path() / outFile;

    // create a .csv file to write to it
    std::fstream file;
    file.open(target_path, std::ios::out | std::ios::app);

    if (!file.is_open())
    {
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

  auto makeDirectory = [&](const fs::path &second_folder)
  {
    fs::path complete_path;

    if (hasSelectedPath)
      complete_path = selectedPath;
    else
    {
      auto tempPath = second_folder;
      tempPath = tempPath.lexically_normal();
      tempPath = tempPath.relative_path();
      auto first_folder = load_json<fs::path>(config, "scanfolder");
      complete_path = first_folder / tempPath;

      // set current directory to ../Omniview
      const std::string temp = fs::current_path();
      const auto pos = temp.find_last_of('/');
      if (temp.substr(pos + 1) != "OmniView")
        fs::current_path(temp.substr(0, pos));

      if (!fs::exists(complete_path))
        fs::create_directories(complete_path);
    }
    return complete_path;
  };

   // to save captureData from main.cpp into file
  if (ImGui::Button(load_json<std::string>(language, "button", "save").c_str(),
                    ImVec2(load_json<Size>(config, "button"))))
  {
    flagDataNotSaved = false;
    std::string filename = fileName(mileage);

    if (captureData.empty())
      ImGui::CloseCurrentPopup();
    else
    {
      auto complete_path = makeDirectory(fs::path(inputvin) / scantype);
      save(captureData, path / complete_path / filename);
      ImGui::CloseCurrentPopup();
    }
  }

  ImGui::SameLine();
  if (ImGui::Button(load_json<std::string>(language, "button", "back").c_str(), {0, 0}))
    ImGui::CloseCurrentPopup();

  fs::path complete_path;
  auto saveDevice = [&]()
  {
    if (path[0] == '\0')
      complete_path = makeDirectory(storagePath_1);
    else
      complete_path = makeDirectory(fs::path(path) / storagePath_1);

    std::string filename = fileName(b1 ? "device1" : "device2");
    saveData(allData, complete_path / filename);
    storagePath_1[0] = 0;
    ImGui::CloseCurrentPopup();
  };

  // if only one of the devices is selected
  if ((b1 || b2) && !(b1 && b2))
  {
    ImGui::SameLine();
    ImGui::Dummy({500, 0});
    ImGui::SameLine();
    if (ImGui::Button("Save Device"))
      saveDevice();
  }

  // if both devices are selected
  else if (b1 && b2)
  {
    ImGui::SameLine();
    ImGui::Dummy({565, 0});
    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 50);

    if (ImGui::Button(" + "))
      ImGui::OpenPopup("StoragePathInputField2");

    ImGui::PopStyleVar();
    ImGui::SetItemTooltip("Add another path");

    ImGui::Dummy({750, 0});
    ImGui::SameLine();
    if (ImGui::Button("Save Device"))
    {
      saveDevice();

      if (path[0] == '\0')
        complete_path = makeDirectory(storagePath_2);
      else
        complete_path = makeDirectory(fs::path(path) / storagePath_2);

      std::string filename = fileName("device2");
      saveData(allData, complete_path / filename);
      storagePath_2[0] = 0;
    }
  }
}
