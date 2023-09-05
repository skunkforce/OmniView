#pragma once

#include <imgui.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include "jasonhandler.hpp"
// clang-format off
#include <imfilebrowser.h>
// clang-format on


void popup_create_training_data_compression(
  nlohmann::json const& config,
  nlohmann::json const& language) {
    static std::string        api_message = " ";
    static ImGui::FileBrowser fileBrowser;
    static ImGui::FileBrowser fileBrowser2;
    static bool               first_job = true;
    static nlohmann::json metadata;

    if(first_job) {
        fileBrowser.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
        fileBrowser2.SetPwd(load_json<std::filesystem::path>(config, "scanfolder"));
        first_job = false;
    }

    static char inputvin[18]  = "";
    static char mileage[10]   = "";
    static char comment[1000] = "";
    ImGui::SetItemDefaultFocus();
    ImGui::BeginChild("trainingleft", ImVec2(500, 300));
    ImGui::Text("stammdaten");
    ImGui::InputText(
      load_json<std::string>(language, "input", "fin", "label").c_str(),
      inputvin,
      sizeof(inputvin));
    ImGui::InputText(
      load_json<std::string>(language, "input", "mileage", "label").c_str(),
      mileage,
      sizeof(mileage));
    ImGui::InputTextMultiline(
      load_json<std::string>(language, "input", "comment", "label").c_str(),
      comment,
      sizeof(comment));

    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("trainingright", ImVec2(500, 300));

    static float z1, z2, z3, z4;
    static char  path1[255];
    static char  path2[255];
    ImGui::Text("Maximalkommpression in bar");
    ImGui::Text("Zylinder 1: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder1", &z1, 0.0f, 0.0f, "%.2f bar");
    ImGui::Text("Zylinder 2: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder2", &z2, 0.0f, 0.0f, "%.2f bar");
    ImGui::Text("Zylinder 3: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder3", &z3, 0.0f, 0.0f, "%.2f bar");
    ImGui::Text("Zylinder 4: ");
    ImGui::SameLine();
    ImGui::InputFloat("##Zylinder4", &z4, 0.0f, 0.0f, "%.2f bar");

    ImGui::Columns(2);
    ImGui::Text("Unterdrückte Zündung");
    ImGui::Text("mit Zündung");
    ImGui::NextColumn();

    ImGui::InputText("##path1", path1, sizeof(path1));
    ImGui::SameLine();
    if(ImGui::Button("File1")) {
        fileBrowser.Open();
    }
    fileBrowser.Display();
    if(fileBrowser.HasSelected()) {
        std::string filepath;
        for(auto const& selectedFile : fileBrowser.GetSelected()) {
            if(!filepath.empty()) {
                filepath += "/";
            }
            filepath += selectedFile.string();
        }

        strcpy(path1, filepath.c_str());

        fileBrowser.ClearSelected();
    }

    ImGui::InputText("##path2", path2, sizeof(path2));
    ImGui::SameLine();
    if(ImGui::Button("File2")) {
        fileBrowser2.Open();
    }
    fileBrowser2.Display();
    if(fileBrowser2.HasSelected()) {
        std::string filepath;
        for(auto const& selectedFile : fileBrowser2.GetSelected()) {
            if(!filepath.empty()) {
                filepath += "/";
            }
            filepath += selectedFile.string();
        }
        strcpy(path2, filepath.c_str());
        fileBrowser2.ClearSelected();
    }
    ImGui::Columns(1);

    if(ImGui::Button(
         load_json<std::string>(language, "button", "send").c_str(),
         ImVec2(load_json<Size>(config, "button"))))
    {
        //Api muss angepasst werden und die funktion send to api ebenso
        metadata["z1"]=z1;
        metadata["z2"]=z2;
        metadata["z3"]=z3;
        metadata["z4"]=z4;
        metadata["kommentar"]=comment;
        metadata["laufleistung"]=mileage;
        metadata["zündung"]="unterdrückt";
        api_message = send_to_api(config, path1, inputvin, "kompressionsmessung",metadata);
        metadata["zündung"]="aktiviert";
        api_message += send_to_api(config, path2, inputvin, "kompressionsmessung",metadata);
    }

    ImGui::SameLine();
    if(ImGui::Button(
         load_json<std::string>(language, "button", "back").c_str(),
         ImVec2(load_json<Size>(config, "button"))))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndChild();
    ImGui::Text(api_message.c_str());
}