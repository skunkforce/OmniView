#pragma once

#include <imgui.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include "jasonhandler.hpp"



void popup_settings(nlohmann::json& config, nlohmann::json& language, std::string const& configpath) {
    static float          fontscale;
    static nlohmann::json newconfig = 0;
    if(newconfig == 0) {
        newconfig = config;
    }
    if(fontscale < load_json<float>(newconfig, "text", "minscale")) {
        fontscale = load_json<float>(newconfig, "text", "scale");
    }
    ImGuiIO& io                 = ImGui::GetIO();
    io.FontGlobalScale          = fontscale;
    std::string fontscalestring = fmt::format(
      "{} {:.1f}",
      load_json<std::string>(language, "settings", "fontsize"),
      fontscale);
    ImGui::Text(fontscalestring.c_str());
    ImGui::SameLine();
    if(ImGui::Button("+")) {
        fontscale += 0.1f;
        newconfig["text"]["scale"] = fontscale;
    }
    ImGui::SameLine();
    if(ImGui::Button("-")) {
        fontscale -= 0.1f;
        newconfig["text"]["scale"] = fontscale;
    }
   
    //####################################################################################
    //                          Button Size
    //####################################################################################
    static float ButtonSizeX = load_json<float>(config, "button", "sizex");
    static float ButtonSizeY = load_json<float>(config, "button", "sizey");

    std::string buttonXsizestring
      = fmt::format("{} {:.1f}", load_json<std::string>(language, "general", "width"), ButtonSizeX);
    std::string buttonYsizestring = fmt::format(
      "{} {:.1f}",
      load_json<std::string>(language, "general", "height"),
      ButtonSizeY);

    ImGui::Text(buttonXsizestring.c_str());

    ImGui::SameLine();
    if(ImGui::Button("X+")) {
        ButtonSizeX += 1.0f;
        newconfig["button"]["sizex"] = ButtonSizeX;
    }

    ImGui::SameLine();
    if(ImGui::Button("X-")) {
        ButtonSizeX -= 1.0f;
        newconfig["button"]["sizex"] = ButtonSizeX;
    }

    ImGui::Text(buttonYsizestring.c_str());
    ImGui::SameLine();
    if(ImGui::Button("Y+")) {
        ButtonSizeY += 1.0f;
        newconfig["button"]["sizey"] = ButtonSizeY;
    }

    ImGui::SameLine();
    if(ImGui::Button("Y-")) {
        ButtonSizeY -= 1.0f;
        newconfig["button"]["sizey"] = ButtonSizeY;
    }

    if(ImGui::Button(
         load_json<std::string>(language, "button", "save").c_str(),
         ImVec2(ButtonSizeX, ButtonSizeY)))
    {
        write_json_file(configpath, newconfig);
        config = newconfig;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if(ImGui::Button(
         load_json<std::string>(language, "button", "cancel").c_str(),
         ImVec2(ButtonSizeX, ButtonSizeY)))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if(ImGui::Button(
         load_json<std::string>(language, "button", "restore").c_str(),
         ImVec2(ButtonSizeX, ButtonSizeY)))
    {
        newconfig   = config;
        fontscale   = load_json<float>(newconfig, "text", "scale");
        ButtonSizeX = load_json<float>(config, "button", "sizex");
        ButtonSizeY = load_json<float>(config, "button", "sizey");
    }
}