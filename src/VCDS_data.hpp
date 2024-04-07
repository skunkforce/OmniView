#pragma once

#include <fmt/format.h>
#include <imgui.h>

#include "look_up_saves.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <future>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <thread>

#include "apihandler.hpp"
#include "jasonhandler.hpp"

// clang-format off
#include "../imgui-filebrowser/imfilebrowser.h"
// clang-format on
static void show_standart_input(nlohmann::json const &config,
                                nlohmann::json &metadata,
                                std::string &inputvin_string,
                                std::string &mileage_string,
                                std::string &comment_string) {
  static std::string inputvin;
  static char mileage[10];
  static char comment[1000];

  ImVec2 windowSize = ImGui::GetWindowSize();
  ImGui::BeginChild("trainingleft",
                    ImVec2(windowSize.x * 0.5f, windowSize.y * 0.8f));
  ImGui::TextUnformatted("base_data");
  inputvin = "jkasldfh";
  ImGui::InputText("label", mileage, sizeof(mileage));
  static bool problem = false;
  static bool maintenance = !problem;

  static bool electric_on = false;
  static bool electric_off = !electric_on;

  static bool expected = true;
  static bool anomaly = !expected;
  ImGui::TextUnformatted("reason");
  ImGui::Checkbox("maintenance", &maintenance);
  if (maintenance == problem) {
    problem = !maintenance;
  }
  ImGui::SameLine();
  ImGui::Checkbox("problem", &problem);
  if (problem == maintenance) {
    maintenance = !problem;
  }

  ImGui::TextUnformatted("electrical consumer");
  ImGui::Checkbox("switched off", &electric_off);
  if (electric_off == electric_on) {
    electric_on = !electric_off;
  }
  ImGui::SameLine();
  ImGui::Checkbox("switched on", &electric_on);
  if (electric_on == electric_off) {
    electric_off = !electric_on;
  }
  ImGui::TextUnformatted("evaluation");
  ImGui::Checkbox("normal_case", &expected);
  if (expected == anomaly) {
    anomaly = !expected;
  }
  ImGui::SameLine();
  ImGui::Checkbox("anomaly", &anomaly);
  if (anomaly == expected) {
    expected = !anomaly;
  }
  ImGui::InputTextMultiline("comment", comment, sizeof(comment));

  ImGui::EndChild();
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
  ImGui::TextUnformatted("vcds");
  static ImGui::FileBrowser fileBrowser;
  static bool first_job = true;
  static bool flagApiSending = false;
  static std::future<std::string> future;

  if (first_job) {
    fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
    first_job = false;
  }

  static char path1[255];

  ImGui::InputText("##path1", path1, sizeof(path1));
  ImGui::SameLine();
  if (ImGui::Button(fmt::format("{} 1", "file").c_str())) {
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
    if (ImGui::Button("send", ImVec2(load_json<Size>(config, "button")))) {
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
      }
      ImGui::CloseCurrentPopup();
    } else {
      ImGui::SameLine();
      ImGui::Text("senden... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    }
  }
  ImGui::EndChild();
}

inline void popup_create_training_data_select(nlohmann::json const &config,
                                              bool &upload_success) {
  static int selectedOption = 0; // Standardauswahl
  const char *options[] = {"VCDS-Datei"};
  ImGui::Combo("Messung", &selectedOption, options, IM_ARRAYSIZE(options));

  static std::string inputvin = "";
  static std::string mileage = "";
  static std::string comment = "";
  static nlohmann::json metadata;
  std::string api_message = " ";
  ImGui::SetItemDefaultFocus();
  show_standart_input(config, metadata, inputvin, mileage, comment);
  ImGui::SameLine();

  ImVec2 windowSize = ImGui::GetWindowSize();
  ImGui::BeginChild("trainingright",
                    ImVec2(windowSize.x * 0.5f, windowSize.y * 0.8f));
  switch (selectedOption) {
  case 0:
    selected_vcds_data(config, metadata, inputvin, mileage, comment,
                       api_message, upload_success);
    break;
  default:
    ImGui::Text("Fehler, Switchcase mit unerwarteter Auswahl");
  }
  if (ImGui::Button("back", ImVec2(load_json<Size>(config, "button")))) {
    ImGui::CloseCurrentPopup();
  }

  // api_message += "something else too";
  // ImGui::TextUnformatted(api_message.c_str());
}
