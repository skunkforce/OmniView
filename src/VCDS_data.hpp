#pragma once

#include <fmt/format.h>
#include <imgui.h>
#include <iostream>

#include "look_up_saves.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <future>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <thread>

#include "apihandler.hpp"
#include "jasonhandler.hpp"
#include "popups.hpp"

// clang-format off
#include "../imgui-filebrowser/imfilebrowser.h"
// clang-format on
static void show_standart_input(nlohmann::json const &config,
                                nlohmann::json &metadata,
                                std::string &inputvin_string,
                                std::string &mileage_string,
                                std::string &comment_string, char *path1) {
  static char inputvin[18];
  static char mileage[10];
  static char comment[1000];

  static ImGui::FileBrowser fileBrowser;
  static bool first_job = true;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  ImGui::Text("Messdaten auswählen");

  ImGui::InputText("##path1", path1, sizeof(path1));
  ImGui::SameLine();
  if (ImGui::Button("Durchsuchen")) {
    fileBrowser.Open();
  }

  fileBrowser.Display();
  if (fileBrowser.HasSelected()) {
    std::string filepath;
    for (auto const &selectedFile : fileBrowser.GetSelected()) {
      if (!filepath.empty()) {
        filepath += "/";
      }
      filepath += selectedFile.string();
    }
    strcpy(path1, filepath.c_str());

    fileBrowser.ClearSelected();
  }
  ImGui::Columns(1);

  ImVec2 windowSize = ImGui::GetWindowSize();
  ImGui::Text("Geben sie die benötigten Eingaben zur Datei ein und wählen sie "
              "ihre Datei aus");
  ImGui::Text(
      "Bitte stellen sie sicher das ihr Gerät mit dem Internet verbunden ist!");

  ImGui::InputText("VIN", inputvin, sizeof(inputvin));
  ImGui::InputText("Kilometerstand", mileage, sizeof(mileage));
  static bool problem = false;
  static bool maintenance = !problem;

  static bool electric_on = false;
  static bool electric_off = !electric_on;

  static bool expected = true;
  static bool anomaly = !expected;
  ImGui::TextUnformatted("Grund");
  ImGui::Checkbox("Wartung", &maintenance);
  if (maintenance == problem) {
    problem = !maintenance;
  }
  ImGui::SameLine();
  ImGui::Checkbox("Problem", &problem);
  if (problem == maintenance) {
    maintenance = !problem;
  }

  ImGui::TextUnformatted("Elektrische Konsument");
  ImGui::Checkbox("aus", &electric_off);
  if (electric_off == electric_on) {
    electric_on = !electric_off;
  }
  ImGui::SameLine();
  ImGui::Checkbox("an", &electric_on);
  if (electric_on == electric_off) {
    electric_off = !electric_on;
  }
  ImGui::TextUnformatted("Bewertung");
  ImGui::Checkbox("Normalfall", &expected);
  if (expected == anomaly) {
    anomaly = !expected;
  }
  ImGui::SameLine();
  ImGui::Checkbox("Anomalie", &anomaly);
  if (anomaly == expected) {
    expected = !anomaly;
  }
  ImGui::InputTextMultiline("Kommentar", comment, sizeof(comment));

  inputvin_string = inputvin;
  mileage_string = mileage;
  comment_string = comment;
  metadata["Elektrische Verbraucher ausgeschaltet:"] = electric_off;
  metadata["Grund des Werkstattbesuchs Wartung"] = maintenance;
  metadata["Anormales Verhalten"] = anomaly;
}

static void select_vcds_data(nlohmann::json const &config, char *PATH2) {

  static ImGui::FileBrowser fileBrowser_VCDS;
  static bool first_job_VCDS = true;

  if (first_job_VCDS) {
    fileBrowser_VCDS.SetPwd(
        load_json<std::filesystem::path>(config, "scanfolder"));
    first_job_VCDS = false;
  }

  ImGui::Text("VCDS Daten auswählen");

  ImGui::InputText("##PATH2", PATH2, sizeof(PATH2));
  ImGui::SameLine();
  if (ImGui::Button("Durchsuche")) {
    fileBrowser_VCDS.Open();
  }

  fileBrowser_VCDS.Display();
  if (fileBrowser_VCDS.HasSelected()) {
    std::string filepath_VCDS;
    for (auto const &selectedFile_VCDS : fileBrowser_VCDS.GetSelected()) {
      if (!filepath_VCDS.empty()) {
        filepath_VCDS += "/";
      }
      filepath_VCDS += selectedFile_VCDS.string();
    }
    strcpy(PATH2, filepath_VCDS.c_str());

    fileBrowser_VCDS.ClearSelected();
  }
  ImGui::Columns(1);
}

inline void send_VCDS_data(nlohmann::json const &config,
                           nlohmann::json &metadata, std::string &inputvin,
                           std::string &mileage, std::string &comment,
                           bool &flagApiSending_VCDS,
                           std::future<std::string> &future_VCDS,
                           bool &wurdegesendet_VCDS, char *path2) {
  if (!flagApiSending_VCDS) {
    wurdegesendet_VCDS = true;
    metadata["kommentar"] = comment;
    metadata["laufleistung"] = mileage;

    future_VCDS = std::async(std::launch::async, [&] {
      std::string result =
          send_to_api(config, path2, inputvin, "vcds", metadata);
      return result;
    });
    flagApiSending_VCDS = true;
  }
}

inline void send_battery_measurement(nlohmann::json const &config,
                                     nlohmann::json &metadata,
                                     std::string &inputvin,
                                     std::string &mileage, std::string &comment,
                                     bool &flagApiSending,
                                     std::future<std::string> &future,
                                     bool &wurdegesendet, char *path1) {
  // Sending Battery Measurement to API
  if (!flagApiSending) {
    wurdegesendet = true;
    metadata["kommentar"] = comment;
    metadata["laufleistung"] = mileage;

    future = std::async(std::launch::async, [&] {
      std::string result =
          send_to_api(config, path1, inputvin, "battery", metadata);
      return result;
    });
    flagApiSending = true;
  }
}

inline void popup_create_training_data_select(nlohmann::json const &config,
                                              bool &upload_success) {

  // Concept: Sending data with : VIN, MILEAGE, MESSAGE (First Version) in PATH1
  // --> Needs to be selected Sending VCDS data to a different API via PATH2
  // with the same sending button --> optional selectable Sending Symptoms to a
  // different API with the same sending button --> optional selectable

  static int selectedOption =
      1; // number of selectable components/ cases (NOT Hub Cases)
  const char *options[] = {"Batteriemessung"}; // possible options
  ImGui::Combo("Messung", &selectedOption, options,
               IM_ARRAYSIZE(options)); // Drop Down Menu

  // Declare variables
  static std::string inputvin = "";
  static std::string mileage = "";
  static std::string comment = "";
  static nlohmann::json metadata;
  static std::string api_message = "";
  static std::string api_message_VCDS = "";
  static std::string complete_api_message = "";
  static char path1[255];
  static char path2[255];
  static bool VCDS_is_selected = false;

  ImGui::SetItemDefaultFocus();

  // setting the first menu part --> Choosing the PATH1 for the data as well as
  // all needed Metadata NOT optional data
  show_standart_input(config, metadata, inputvin, mileage, comment, path1);

  // Selecting VCDS data

  ImGui::Checkbox("VCDS", &VCDS_is_selected);
  if (VCDS_is_selected) {
    select_vcds_data(config, path2);
  }

  // Sending data dependend on the chosen options
  static bool flagApiSending_VCDS = false;
  static std::future<std::string> future_VCDS;
  static bool wurdegesendet_VCDS = false;

  static bool flagApiSending = false;
  static std::future<std::string> future;
  static bool wurdegesendet = false;
  static bool test = false;

  static bool data_was_send =
      false; // to check if the data was send before the api_message popsup ,
             // this is important because otherwise the data could be send after
             // the VCDS data and the popup wont show the right message

  static bool VCDS_was_send = false; // to check if the VCDS was send before the
                                     // api_message popsup, see comment above

  using namespace std::chrono_literals;

  if (ImGui::Button("SENDEN", ImVec2(load_json<Size>(config, "button")))) {
    switch (selectedOption) {
    case 0:
      send_battery_measurement(config, metadata, inputvin, mileage, comment,
                               flagApiSending, future, wurdegesendet, path1);
      break;
    default:
      ImGui::Text("Fehler, Switchcase mit unerwarteter Auswahl");
    }
    if (VCDS_is_selected) {
      send_VCDS_data(config, metadata, inputvin, mileage, comment,
                     flagApiSending_VCDS, future_VCDS, wurdegesendet_VCDS,
                     path2);
    }
  }

  if (flagApiSending) {
    ImGui::PushStyleColor(
        ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
    ImGui::PopStyleColor();
    auto status = future.wait_for(10ms);
    if (status == std::future_status::ready) {
      upload_success = true;
      flagApiSending = false;
      if (future.valid()) {
        api_message = future.get();
        data_was_send = true;
        if (VCDS_is_selected) {
          if (wurdegesendet && VCDS_was_send) {
            ImGui::OpenPopup("message1");
            wurdegesendet = false;
            VCDS_was_send = false;
          }
        } else {
          if (wurdegesendet) {
            ImGui::OpenPopup("message");
            wurdegesendet = false;
            data_was_send = false;
          }
        }
      }
      // ImGui::CloseCurrentPopup();
    } else {
      ImGui::SameLine();
      ImGui::Text("senden... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    }
  }

  if (flagApiSending_VCDS) {
    ImGui::PushStyleColor(
        ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
    ImGui::PopStyleColor();
    auto status = future_VCDS.wait_for(10ms);
    if (status == std::future_status::ready) {
      upload_success = true;
      flagApiSending_VCDS = false;
      if (future_VCDS.valid()) {
        api_message_VCDS = future_VCDS.get();
        VCDS_was_send = true;
        if (wurdegesendet_VCDS && data_was_send) {
          ImGui::OpenPopup("message1");
          wurdegesendet_VCDS = false;
          data_was_send = false;
        }
      }
      // ImGui::CloseCurrentPopup();
    } else {
      ImGui::SameLine();
      ImGui::Text("senden... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    }
  }

  // Open API Message
  complete_api_message = api_message + " ." + api_message_VCDS;
  info_popup_test("message1", complete_api_message.c_str());
  info_popup_test("message", api_message.c_str());

  ImGui::SameLine();
  if (ImGui::Button("schließen", ImVec2(load_json<Size>(config, "button")))) {
    ImGui::CloseCurrentPopup();
  }
}