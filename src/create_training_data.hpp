#pragma once

#include "jasonhandler.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

// clang-format off
#include <imfilebrowser.h>
// clang-format on

void show_standart_input(nlohmann::json const &language,
                         nlohmann::json &metadata, std::string &inputvin_string,
                         std::string &mileage_string,
                         std::string &comment_string) {
  static char inputvin[18];
  static char mileage[10];
  static char comment[1000];

  ImVec2 windowSize = ImGui::GetWindowSize();
  ImGui::BeginChild("trainingleft",
                    ImVec2(windowSize.x * 0.5f, windowSize.y * 0.8f));
  ImGui::Text(
      load_json<std::string>(language, "training", "base_data").c_str());
  ImGui::InputText(
      load_json<std::string>(language, "input", "fin", "label").c_str(),
      inputvin, sizeof(inputvin));
  ImGui::InputText(
      load_json<std::string>(language, "input", "mileage", "label").c_str(),
      mileage, sizeof(mileage));
  static bool problem = false;
  static bool maintenance = !problem;

  static bool electric_on = false;
  static bool electric_off = !electric_on;

  static bool expected = true;
  static bool anomaly = !expected;
  ImGui::Text(load_json<std::string>(language, "training", "reason").c_str());
  ImGui::Checkbox(
      load_json<std::string>(language, "training", "maintenance").c_str(),
      &maintenance);
  if (maintenance == problem) {
    problem = !maintenance;
  }
  ImGui::SameLine();
  ImGui::Checkbox(
      load_json<std::string>(language, "training", "problem").c_str(),
      &problem);
  if (problem == maintenance) {
    maintenance = !problem;
  }

  ImGui::Text(
      load_json<std::string>(language, "training", "electrical_consumer")
          .c_str());
  ImGui::Checkbox(
      load_json<std::string>(language, "training", "switched_off").c_str(),
      &electric_off);
  if (electric_off == electric_on) {
    electric_on = !electric_off;
  }
  ImGui::SameLine();
  ImGui::Checkbox(
      load_json<std::string>(language, "training", "switched_on").c_str(),
      &electric_on);
  if (electric_on == electric_off) {
    electric_off = !electric_on;
  }
  ImGui::Text(
      load_json<std::string>(language, "training", "evaluation").c_str());
  ImGui::Checkbox(
      load_json<std::string>(language, "training", "normal_case").c_str(),
      &expected);
  if (expected == anomaly) {
    anomaly = !expected;
  }
  ImGui::SameLine();
  ImGui::Checkbox(
      load_json<std::string>(language, "training", "anomaly").c_str(),
      &anomaly);
  if (anomaly == expected) {
    expected = !anomaly;
  }
  ImGui::InputTextMultiline(
      load_json<std::string>(language, "input", "comment", "label").c_str(),
      comment, sizeof(comment));

  ImGui::EndChild();
  inputvin_string = inputvin;
  mileage_string = mileage;
  comment_string = comment;
  metadata["Elektrische Verbraucher ausgeschaltet:"] = electric_off;
  metadata["Grund des Werkstattbesuchs Wartung"] = maintenance;
  metadata["Anormales Verhalten"] = anomaly;
}
void selected_vcds_data(nlohmann::json const &config,
                        nlohmann::json const &language,
                        nlohmann::json &metadata, std::string &inputvin,
                        std::string &mileage, std::string &comment,
                        std::string &api_message, bool &upload_success) {
  ImGui::Text(load_json<std::string>(language, "explanation", "upload", "vcds")
                  .c_str());
  static ImGui::FileBrowser fileBrowser;
  static bool first_job = true;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  static char path1[255];

  ImGui::InputText("##path1", path1, sizeof(path1));
  ImGui::SameLine();
  if (ImGui::Button("File1")) {
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

  if (ImGui::Button(load_json<std::string>(language, "button", "send").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
    // Api muss angepasst werden und die funktion send to api ebenso

    metadata["kommentar"] = comment;
    metadata["laufleistung"] = mileage;
    api_message = send_to_api(
        config, path1, inputvin,
        load_json<std::string>(language, "measuretype", "vcds"), metadata);
  }

  ImGui::SameLine();
  if (ImGui::Button(load_json<std::string>(language, "button", "back").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {

    ImGui::CloseCurrentPopup();
  }
}
void selected_battery_measurement(
    nlohmann::json const &config, nlohmann::json const &language,
    nlohmann::json &metadata, std::string &inputvin, std::string &mileage,
    std::string &comment, std::string &api_message, bool &upload_success) {
  ImGui::Text(
      load_json<std::string>(language, "explanation", "upload", "battery")
          .c_str());
  static ImGui::FileBrowser fileBrowser;
  static bool first_job = true;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  static char path1[255];

  ImGui::InputText("##path1", path1, sizeof(path1));
  ImGui::SameLine();
  if (ImGui::Button("File1")) {
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

  if (ImGui::Button(load_json<std::string>(language, "button", "send").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
    // Api muss angepasst werden und die funktion send to api ebenso

    metadata["kommentar"] = comment;
    metadata["laufleistung"] = mileage;
    api_message = send_to_api(
        config, path1, inputvin,
        load_json<std::string>(language, "measuretype", "battery"), metadata);
    upload_success = true;
    ImGui::CloseCurrentPopup();
  }

  ImGui::SameLine();
  if (ImGui::Button(load_json<std::string>(language, "button", "back").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {

    ImGui::CloseCurrentPopup();
  }
}
void selected_compression_data(nlohmann::json const &config,
                               nlohmann::json const &language,
                               nlohmann::json &metadata, std::string &inputvin,
                               std::string &mileage, std::string &comment,
                               std::string &api_message, bool &upload_success) {

  static ImGui::FileBrowser fileBrowser;
  static ImGui::FileBrowser fileBrowser2;
  static bool first_job = true;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    fileBrowser2.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  static float z1, z2, z3, z4;
  static char path1[255];
  static char path2[255];
  const std::string cylinder =
      load_json<std::string>(language, "training", "compression", "cylinder");

  ImGui::Text(load_json<std::string>(language, "training", "compression",
                                     "max_compression")
                  .c_str());

  ImGui::Text(cylinder.c_str());
  ImGui::SameLine();
  ImGui::Text("1:");
  ImGui::SameLine();
  ImGui::InputFloat("##Zylinder1", &z1, 0.0f, 0.0f, "%.2f bar");
  ImGui::Text(cylinder.c_str());
  ImGui::SameLine();
  ImGui::Text("2:");
  ImGui::SameLine();
  ImGui::InputFloat("##Zylinder2", &z2, 0.0f, 0.0f, "%.2f bar");
  ImGui::Text(cylinder.c_str());
  ImGui::SameLine();
  ImGui::Text("3:");
  ImGui::SameLine();
  ImGui::InputFloat("##Zylinder3", &z3, 0.0f, 0.0f, "%.2f bar");
  ImGui::Text(cylinder.c_str());
  ImGui::SameLine();
  ImGui::Text("4:");
  ImGui::SameLine();
  ImGui::InputFloat("##Zylinder4", &z4, 0.0f, 0.0f, "%.2f bar");

  ImGui::Columns(2);
  ImGui::Text(load_json<std::string>(language, "training", "compression",
                                     "ignition_suspended")
                  .c_str());
  ImGui::Text(load_json<std::string>(language, "training", "compression",
                                     "ignition_active")
                  .c_str());
  ImGui::NextColumn();

  ImGui::InputText("##path1", path1, sizeof(path1));
  ImGui::SameLine();
  const std::string button1 =
      load_json<std::string>(language, "button", "file") + " 1";
  const std::string button2 =
      load_json<std::string>(language, "button", "file") + " 2";
  if (ImGui::Button(button1.c_str())) {
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

  ImGui::InputText("##path2", path2, sizeof(path2));
  ImGui::SameLine();
  if (ImGui::Button(button2.c_str())) {
    fileBrowser2.Open();
  }
  fileBrowser2.Display();
  if (fileBrowser2.HasSelected()) {
    std::string filepath;
    for (auto const &selectedFile : fileBrowser2.GetSelected()) {
      if (!filepath.empty()) {
        filepath += "/";
      }
      filepath += selectedFile.string();
    }
    strcpy(path2, filepath.c_str());

    fileBrowser2.ClearSelected();
  }
  ImGui::Columns(1);

  if (ImGui::Button(load_json<std::string>(language, "button", "send").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
    // Api muss angepasst werden und die funktion send to api ebenso
    metadata["z1"] = z1;
    metadata["z2"] = z2;
    metadata["z3"] = z3;
    metadata["z4"] = z4;
    metadata["kommentar"] = comment;
    metadata["laufleistung"] = mileage;
    metadata["zündung"] = "unterdrückt";
    api_message = send_to_api(
        config, path1, inputvin,
        load_json<std::string>(language, "measuretype", "compression"),
        metadata);
    metadata["zündung"] = "aktiviert";
    api_message += send_to_api(
        config, path2, inputvin,
        load_json<std::string>(language, "measuretype", "compression"),
        metadata);
    upload_success = true;
    ImGui::CloseCurrentPopup();
  }

  ImGui::SameLine();
  if (ImGui::Button(load_json<std::string>(language, "button", "back").c_str(),
                    ImVec2(load_json<Size>(config, "button")))) {
    ImGui::CloseCurrentPopup();
  }
}

void popup_create_training_data_select(nlohmann::json const &config,
                                       nlohmann::json const &language,
                                       bool &upload_success) {

  static int selectedOption = 0; // Standardauswahl
  /*
  load_json<std::string>(language, "measuretype", "compression").c_str(),
  load_json<std::string>(language, "measuretype", "battery").c_str(),
  load_json<std::string>(language, "measuretype", "vcds").c_str()
  */
  const char *options[] = {"Kompressionsmessung", "Batteriemessung",
                           "VCDS-Datei"};
  ImGui::Combo("Messung", &selectedOption, options, IM_ARRAYSIZE(options));

  static std::string inputvin = "";
  static std::string mileage = "";
  static std::string comment = "";
  static nlohmann::json metadata;
  static std::string api_message = " ";
  ImGui::SetItemDefaultFocus();
  show_standart_input(language, metadata, inputvin, mileage, comment);
  ImGui::SameLine();

  ImVec2 windowSize = ImGui::GetWindowSize();
  ImGui::BeginChild("trainingright",
                    ImVec2(windowSize.x * 0.5f, windowSize.y * 0.8f));
  switch (selectedOption) {
  case 0:
    selected_compression_data(config, language, metadata, inputvin, mileage,
                              comment, api_message, upload_success);
    break;
  case 1:
    selected_battery_measurement(config, language, metadata, inputvin, mileage,
                                 comment, api_message, upload_success);
    break;
  case 2:
    selected_vcds_data(config, language, metadata, inputvin, mileage, comment,
                       api_message, upload_success);
    break;
  default:
    ImGui::Text("Fehler, Switchcase mit unerwarteter Auswahl");
  }

  ImGui::EndChild();
  ImGui::Text(api_message.c_str());
}