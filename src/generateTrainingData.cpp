#include <fstream>
#include <regex>
#include <imgui.h>
#include <filesystem>
#include <sstream>
#include <limits>
#include <future>
#include <iostream>
#include <cstring>
#include "languages.hpp"
#include "popups.hpp"
#include "sendData.hpp"
#include "../imgui-stdlib/imgui_stdlib.h"
#include "jasonhandler.hpp"

void generateTrainingData(
    bool &open_generate_training_data,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData,
    std::set<std::string> &savedFileNames) {

  ImGui::OpenPopup(appLanguage[Key::Gn_trng_data_pop]);
  if (ImGui::BeginPopupModal(appLanguage[Key::Gn_trng_data_pop], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    static bool usr_curnt_wave{false};
    static bool wave_from_file{false};
    static bool callSetInptFields{false};
    static bool resetInptFields{false};
    static std::string selected_file;
    static std::string api_message;

    if (ImGui::RadioButton(appLanguage[Key::Usr_curnt_wave], usr_curnt_wave)) {
      usr_curnt_wave = !usr_curnt_wave;
      wave_from_file = false;
      // set inputfields whenever usr_curnt_wave is set and there's a selected
      // file
      if (usr_curnt_wave && !selected_file.empty()) {
        resetInptFields = true;
        callSetInptFields = true;
      }
    }

    if (usr_curnt_wave && !captureData.size() && !savedFileNames.size()) {
      ImGui::OpenPopup(appLanguage[Key::WvForms_warning],
                       ImGuiPopupFlags_NoOpenOverExistingPopup);
      usr_curnt_wave = false;
    }
    info_popup(appLanguage[Key::WvForms_warning],
               appLanguage[Key::No_wave_made]);

    static Omniscope::Id selected_device{};
    static int token{-1};

    if (usr_curnt_wave && (captureData.size() || savedFileNames.size()) &&
        (ImGui::BeginCombo("##ComboDevice", "Devices & Waveforms Menu"))) {
      // only one checkbox out of two sets of
      // devices/waveforms is selected at any time
      int i{};
      bool is_checked{false};
      if (captureData.size())
        for (const auto &[device, values] : captureData) {
          is_checked = (token == i);
          if (ImGui::Checkbox(device.serial.c_str(), &is_checked)) {
            if (is_checked) {
              token = i;
              selected_device = device;
              resetInptFields = true;
            } else {
              token = -1;
              selected_device = {}; // reset
            }
          }
          i++;
        }
      i = 0;
      if (const size_t sz = savedFileNames.size())
        for (const auto &file : savedFileNames) {
          is_checked = (token == i + sz);
          if (ImGui::Checkbox(file.c_str(), &is_checked)) {
            if (is_checked) {
              token = i + sz;
              selected_file = file.c_str();
              resetInptFields = true;
              callSetInptFields = true;
            } else {
              token = -1;
              resetInptFields = true;
              selected_file.clear();
            }
          }
          ++i;
        } // End of the algorithm
      ImGui::EndCombo();
    }
    static std::string Measurement;
    // one extra space for '\0' character
    static char VIN[19];
    static std::string Mileage;
    static std::vector<double> file_measuring_vals; // measurement values
    static bool grayFields = false;
    // custom flags
    static auto measuGrayFlag = ImGuiInputTextFlags_None;
    static auto vinGrayFlag = ImGuiInputTextFlags_None;
    static auto milGrayFlag = ImGuiInputTextFlags_None;

    auto setInptFields = [&](const fs::path &filename) {
      constexpr size_t fieldsSize{3}; // Measurement, Vin and Mileage
      std::vector<std::string> FieldsData(fieldsSize);
      std::ifstream readfile(filename, std::ios::binary);

      if (!readfile.is_open())
        fmt::println("Failed to open file {}", filename);
      else {
        // first line of file
        std::string first_line;
        std::getline(readfile, first_line);
        first_line.pop_back(); // remove ending new line
        std::stringstream ss(first_line);
        // extract input fields data from the first line
        for (size_t j = 0; j < fieldsSize; j++) {
          std::string substr;
          std::getline(ss, substr, ',');
          FieldsData[j] = substr;
        }
        while (!readfile.eof()) {
          // read measuring values into the vector
          double value{};
          readfile >> value;
          file_measuring_vals.emplace_back(value);
          static constexpr size_t bigNumber{10'000'000};
          readfile.ignore(bigNumber,
                          '\n'); // new line separator between elements
          // at the last loop, the last number is picked, loop
          // goes on and vector pushes value before eof is reached
        }
        // pop the extra last element
        file_measuring_vals.pop_back();

        Measurement = FieldsData[0];
        if (!Measurement.empty())
          measuGrayFlag = ImGuiInputTextFlags_ReadOnly;

        std::memset(VIN, 0, sizeof VIN); // reset char array containing old data
        FieldsData[1].copy(VIN, FieldsData[1].size());
        if (VIN[0] != 0)
          vinGrayFlag = ImGuiInputTextFlags_ReadOnly;

        Mileage = FieldsData[2];
        if (!Mileage.empty())
          milGrayFlag = ImGuiInputTextFlags_ReadOnly;
        grayFields = true;
      }
    };

    static std::string fileNameBuf;
    static std::string analyzedWavefile;
    auto isWrongFile = [&](const fs::path &path) {
      if (path.extension() == ".csv") {
        fileNameBuf = path.string();
        analyzedWavefile = path.filename().string();
        savedFileNames.insert(fileNameBuf);
        measuGrayFlag = ImGuiInputTextFlags_None;
        vinGrayFlag = ImGuiInputTextFlags_None;
        milGrayFlag = ImGuiInputTextFlags_None;
        setInptFields(path);
        return false;
      }
      return true;
    };

    if (resetInptFields) {
      grayFields = false;
      Measurement.clear();
      std::memset(VIN, 0, sizeof VIN); // reset char array
      Mileage.clear();
      resetInptFields = false;
    }
    if (callSetInptFields) {
      setInptFields(selected_file);
      callSetInptFields = false;
    }

    static ImGui::FileBrowser fileBrowser;
    fileBrowser.SetTitle("Searching for .csv files"); // properties
    // fileBrowser.SetTypeFilters({".csv"});
    fileBrowser.Display();
    if (fileBrowser.HasSelected()) {
      if (isWrongFile(fileBrowser.GetSelected().string())) {
        ImGui::OpenPopup(appLanguage[Key::Wrong_file_warning],
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
        wave_from_file = false;
      }
      fileBrowser.ClearSelected();
    }
    info_popup(appLanguage[Key::Wrong_file_warning],
               appLanguage[Key::Wrong_file_type]);

    if (ImGui::RadioButton(appLanguage[Key::Wv_from_file], wave_from_file)) {
      wave_from_file = !wave_from_file;
      usr_curnt_wave = false;
      if (wave_from_file && !fileNameBuf.empty())
        setInptFields(fileNameBuf);
    }

    static constexpr ImVec4 greyBtnStyle{0.5f, 0.5f, 0.5f, 1.0f};

    if (wave_from_file) {
      if (grayFields) // grey color style      rgba values
        ImGui::PushStyleColor(ImGuiCol_Text, greyBtnStyle);

      ImGui::SetNextItemWidth(400); // custom width
      ImGui::InputTextWithHint("##inputLabel", appLanguage[Key::Csv_file],
                               &fileNameBuf, ImGuiInputTextFlags_ReadOnly);
      if (grayFields)
        ImGui::PopStyleColor(); // remove grey color style

      ImGui::SameLine();
      if (ImGui::Button(appLanguage[Key::Browse]))
        fileBrowser.Open();
    }

    if (!usr_curnt_wave && !wave_from_file)
      resetInptFields = true;

    auto vinFilter = [](ImGuiInputTextCallbackData *data) -> int {
      const std::regex chars_regex("[A-HJ-NPR-Z0-9]");
      std::string s;
      // get entered char and save it into string
      s += data->EventChar;
      // strlen is updated when entered char passes the filter
      size_t indx = strlen(VIN);
      if (indx < 18)
        return !std::regex_match(
            s, chars_regex); // return 0 as passed for matched chars
      return 1;              // discard exceeding chars
    };

    if (grayFields) // greyed out color style
      ImGui::PushStyleColor(ImGuiCol_Text, greyBtnStyle);

    static std::string ID;
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("ID", appLanguage[Key::Set_id_in_setting], &ID,
                             ImGuiInputTextFlags_ReadOnly);

    ImGui::SetNextItemWidth(400);
    static std::string measurHint{appLanguage[Key::Enter_measurement]};
    ImGui::InputTextWithHint(appLanguage[Key::Measurement], measurHint.c_str(),
                             &Measurement, measuGrayFlag);
    if (ImGui::IsItemFocused())
      measurHint.clear();
    else
      measurHint = appLanguage[Key::Enter_measurement];

    ImGui::SetNextItemWidth(400);
    static std::string vinHint{appLanguage[Key::Enter_vin]};
    ImGui::InputTextWithHint(
        "VIN", vinHint.c_str(), VIN, IM_ARRAYSIZE(VIN),
        ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank |
            ImGuiInputTextFlags_CallbackCharFilter | vinGrayFlag,
        // callback function to filter each character
        // before putting it into the buffer
        vinFilter);
    if (ImGui::IsItemFocused())
      vinHint.clear();
    else
      vinHint = appLanguage[Key::Enter_vin];

    ImGui::SetNextItemWidth(400);
    static std::string milHint{appLanguage[Key::Enter_mileage]};
    ImGui::InputTextWithHint(appLanguage[Key::Mileage], milHint.c_str(),
                             &Mileage, milGrayFlag);
    if (ImGui::IsItemFocused())
      milHint.clear();
    else
      milHint = appLanguage[Key::Enter_mileage];

    if (grayFields)
      ImGui::PopStyleColor(); // remove grey color style

    ImGui::SeparatorText(appLanguage[Key::Invst_reason]);
    static int invest = 0;
    ImGui::RadioButton(appLanguage[Key::Maintenance], &invest, 0);
    ImGui::SameLine();
    ImGui::RadioButton(appLanguage[Key::Fault], &invest, 1);
    ImGui::SeparatorText(appLanguage[Key::Electr_consumer]);
    static int consumer = 0;
    ImGui::RadioButton(appLanguage[Key::Off], &consumer, 0);
    ImGui::SameLine();
    ImGui::RadioButton(appLanguage[Key::On], &consumer, 1);
    ImGui::SeparatorText(appLanguage[Key::Assessment]);
    static int assess = 0;
    ImGui::RadioButton("Normal", &assess, 0);
    ImGui::SameLine();
    ImGui::RadioButton(appLanguage[Key::Anomaly], &assess, 1);
    static std::string comment;
    ImGui::InputTextMultiline(appLanguage[Key::Comment], &comment,
                              ImVec2(350, 70),
                              ImGuiInputTextFlags_AllowTabInput);

    auto clearSettings = [&]() {
      usr_curnt_wave = false;
      wave_from_file = false;
      ID.clear();
      Measurement.clear();
      std::memset(VIN, 0, sizeof VIN);
      Mileage.clear();
      token = -1;           // reset checkboxes
      selected_device = {}; // reset the content
      selected_file.clear();
      fileNameBuf.clear();
      invest = consumer = assess = 0; // reset radio buttons
      comment.clear();
      grayFields = false;
      measuGrayFlag = ImGuiInputTextFlags_None;
      vinGrayFlag = ImGuiInputTextFlags_None;
      milGrayFlag = ImGuiInputTextFlags_None;
    };

    if (ImGui::Button(appLanguage[Key::Back])) {
      clearSettings();
      open_generate_training_data = false;
      ImGui::CloseCurrentPopup();
    }

    static bool flagApiSending = false;
    static std::future<std::string> future;
    using namespace std::chrono_literals;
    const bool grayStyle = flagApiSending;
    ImGui::SameLine();

    if (grayStyle)
      ImGui::PushStyleColor(ImGuiCol_Text, greyBtnStyle);

    if (ImGui::Button(appLanguage[Key::Send]) && !flagApiSending) {
      std::vector<double> crnt_measuring_vals;
      bool has_selection{false};

      if (usr_curnt_wave) {
        if (!selected_device.serial.empty()) {
          auto it{captureData.find(selected_device)};
          if (it != captureData.end()) {
            // read measuring values from the wave into the vector
            crnt_measuring_vals.resize(it->second.size());
            for (size_t i = 0; i < it->second.size(); ++i)
              crnt_measuring_vals[i] = it->second[i].second;

            has_selection = true;
          } else
            fmt::println("Selected device {} is not found!",
                         selected_device.serial);
        } else if (!selected_file.empty()) {
          setInptFields(selected_file);
          has_selection = true;
        }
      } else if (wave_from_file && !fileNameBuf.empty())
        has_selection = true;

      if (!has_selection)
        ImGui::OpenPopup(appLanguage[Key::Nothing_to_send],
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
      else {
        std::string invest_reason, elec_consumer, assessmnt;
        if (invest)
          invest_reason = appLanguage[Key::Fault];
        else
          invest_reason = appLanguage[Key::Maintenance];
        if (consumer)
          elec_consumer = appLanguage[Key::On];
        else
          elec_consumer = appLanguage[Key::Off];
        if (assess)
          assessmnt = appLanguage[Key::Anomaly];
        else
          assessmnt = "Normal";

        std::vector<double> y_values;
        if (usr_curnt_wave)
          y_values = std::move(crnt_measuring_vals);
        else
          y_values = std::move(file_measuring_vals);

        static nlohmann::ordered_json myJson;
        myJson["meta"] = {Measurement, VIN, Mileage, invest_reason,
                          elec_consumer, assessmnt, comment};
        myJson["data"] = {{"sampling_rate", 100000}, {"y_values", y_values}};

        // Optional - see what you've uploaded
        fs::path outFile = fs::current_path() / "myJson.json";
        std::ofstream writefile(outFile, std::ios::trunc);
        if (!writefile.is_open()) {
          writefile.clear();
          fmt::println("Could not create {} for writing!", outFile.string());
        } else {
          fmt::println("Start saving {}.", outFile.string());
          writefile << myJson;
          writefile.flush();
          writefile.close();
          fmt::println("Finished saving json file.");
        }

        // upload data asynchronously using a separate thread
        future = std::async(std::launch::async, [&] {
          // take temp object returned from dump() and send it to sendData
          std::string result = sendData(myJson.dump());
          return result;
        });

        flagApiSending = true;
      }
    }
    info_popup(appLanguage[Key::Nothing_to_send],
               appLanguage[Key::No_slct_to_upld]);

    if (grayStyle)
      ImGui::PopStyleColor();

    static bool analyze{false};
    if (flagApiSending) {
      auto status = future.wait_for(10ms);
      if (status == std::future_status::ready) {
        ImGui::OpenPopup(appLanguage[Key::Data_upload],
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
        flagApiSending = false;
        if (future.valid()) {
          api_message = future.get();
          analyze = true;
          clearSettings();
        }
      } else {
        ImGui::SameLine();
        ImGui::Text(" sending ... %c",
                    "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
      }
    }
    info_popup(appLanguage[Key::Data_upload],
               api_message == "empty" ? appLanguage[Key::Upload_failure]
                                      : appLanguage[Key::Upload_success]);

    if (analyze && api_message != "empty") {
      fs::path complete_path = fs::current_path() / "analyze";
      if (!fs::exists(complete_path))
        fs::create_directories(complete_path);

      fs::path outFile = complete_path / ("Analysis_" + analyzedWavefile);

      std::ofstream writefile(outFile, std::ios::trunc);
      if (!writefile.is_open()) {
        writefile.clear();
        fmt::println("Could not create {} for writing!", outFile.string());
      } else {
        fmt::println("Start saving {}.", outFile.string());
        writefile << api_message;
        writefile.flush();
        writefile.close();
        fmt::println("Finished saving CSV file.");
      }
      analyze = false;
    }
    ImGui::EndPopup();
  }
}