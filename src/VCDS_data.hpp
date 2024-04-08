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
                                std::string &comment_string) {
  static char inputvin[18];
  static char mileage[10];
  static char comment[1000];

  ImVec2 windowSize = ImGui::GetWindowSize();
  ImGui::Text("Geben sie die benötigten Eingaben zur Datei ein und wählen sie "
              "ihre Datei aus");
  ImGui::Text(
      "BITTE STELLEN SIE SICHER DAS IHR GERÄT MIT DEM INTERNET VERBUNDEN IST!");

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

static void selected_vcds_data(nlohmann::json const &config,
                               nlohmann::json &metadata, std::string &inputvin,
                               std::string &mileage, std::string &comment,
                               std::string &api_message, bool &upload_success) {

  static ImGui::FileBrowser fileBrowser;
  static bool first_job = true;
  static bool flagApiSending = false;
  static std::future<std::string> future;
  static bool wurdegesendet = false;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  static char path1[255];

  ImGui::Text("VCDS Datei auswählen");

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

  using namespace std::chrono_literals;
  if (!flagApiSending) {
    if (ImGui::Button("senden", ImVec2(load_json<Size>(config, "button")))) {
      wurdegesendet = true;
      metadata["kommentar"] = comment;
      metadata["laufleistung"] = mileage;

      future = std::async(std::launch::async, [&] {
        std::string result =
            send_to_api(config, path1, inputvin, "vcds", metadata);
        return result;
      });
      flagApiSending = true;
    }
  }
  if (flagApiSending) {
    ImGui::PushStyleColor(
        ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
    if (ImGui::Button("send", ImVec2(load_json<Size>(config, "button")))) {
    }
    ImGui::PopStyleColor();
    auto status = future.wait_for(10ms);
    if (status == std::future_status::ready) {
      upload_success = true;
      flagApiSending = false;
      if (future.valid()) {
        api_message = future.get();
        if (wurdegesendet) {
         ImGui::OpenPopup("message");
        wurdegesendet = false;
        }
      }
      // ImGui::CloseCurrentPopup();
    } else {
      ImGui::SameLine();
      ImGui::Text("senden... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    }
    info_popup_test("message", api_message);
  }
}

inline void
selected_battery_measurement(nlohmann::json const &config,
                             nlohmann::json &metadata, std::string &inputvin,
                             std::string &mileage, std::string &comment,
                             std::string &api_message, bool &upload_success) {

  static ImGui::FileBrowser fileBrowser;
  static bool first_job = true;
  static bool flagApiSending = false;
  static std::future<std::string> future;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  static char path1[255];

  ImGui::Text("Batterie Messung auswählen");

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
  using namespace std::chrono_literals;
  if (!flagApiSending) {
    if (ImGui::Button("senden", ImVec2(load_json<Size>(config, "button")))) {
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
  if (flagApiSending) {
    ImGui::PushStyleColor(
        ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
    if (ImGui::Button("senden", ImVec2(load_json<Size>(config, "button")))) {
    }
    ImGui::PopStyleColor();
    auto status = future.wait_for(10ms);
    if (status == std::future_status::ready) {
      upload_success = true;
      flagApiSending = false;
      if (future.valid()) {
        api_message = future.get();
      }
      // ImGui::CloseCurrentPopup();
    } else {
      ImGui::SameLine();
      ImGui::Text("senden... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    }
  }
}

inline void popup_create_training_data_select(nlohmann::json const &config,
                                              bool &upload_success) {

  static int selectedOption = 1; // Standardauswahl

  const char *options[] = {"VCDS-Datei", "Batteriemessung"};
  ImGui::Combo("Messung", &selectedOption, options, IM_ARRAYSIZE(options));

  static std::string inputvin = "";
  static std::string mileage = "";
  static std::string comment = "";
  static nlohmann::json metadata;
  static std::string api_message;
  ImGui::SetItemDefaultFocus();
  show_standart_input(config, metadata, inputvin, mileage, comment);

  switch (selectedOption) {
  case 0:
    selected_vcds_data(config, metadata, inputvin, mileage, comment,
                       api_message, upload_success);
    break;
  case 1:
    selected_battery_measurement(config, metadata, inputvin, mileage, comment,
                                 api_message, upload_success);
    break;
  }
  ImGui::SameLine();
  if (ImGui::Button("schließen", ImVec2(load_json<Size>(config, "button")))) {
    ImGui::CloseCurrentPopup();
  }

  // api_message += "something else too";
  // ImGui::TextUnformatted(api_message.c_str());
}
