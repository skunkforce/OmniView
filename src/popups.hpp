// popups.hpp

#ifndef POPUPS_HEADER_HPP
#define POPUPS_HEADER_HPP

#include <imgui.h>
#include <string>
#include <set>
#include <memory>
#include <algorithm>
#include <map>
#include "imgui_stdlib.h"
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeDevice.hpp"
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "../imgui-filebrowser/imfilebrowser.h"

  // Have address of bool for std::vector
    struct BoolWrapper {
      BoolWrapper() : b(false) {}
      BoolWrapper(bool _b) : b(_b) {}
      bool b;
    };

   static std::map<const char*, const char*> englishLan {
        {"Menu", "Menu"},
        {"LanOption", "Language Option"},
        {"English", "English"},
        {"Settings", "Settings"},
        {"Reset", "Reset"},
        {"Version", "Version"},
        {"Diagnostics", "Diagnostics"},
        {"Compression", "Compression"},
        {"Anlyz_crnt_waveform", "Analyze current waveform"},
        {"Gnrt_trning_data", "Generate training data"},
        {"Timing-Belt", "Timing-Belt"},
        {"Fuel-Delivery-Pump", "Fuel-Delivery-Pump"},
        {"Common-Rail-Pressure", "Common-Rail-Pressure"},
        {"Help", "Help"},
        {"HelpLink", "Click on the button to go to the help page"},
        {"Dvc_search", "Search for\nDevices"},
        {"Save", "Save"},
        {"Save warning", "Save warning"},
        {"No_dvc_available", "No devices are available ..."},
        {"AnalyzeData", "Analyze\nData"},
        {"Crt_trng_data", "Create\nTraining\nData"},
        {"Back", "Back"} };

   static std::map<const char*, const char*> germanLan {
       {"Menu", "Menü"},
       {"LanOption", "Sprachoption"},
       {"English", "Englisch"},
       {"Settings", "Einstellungen"},
       {"Reset", "Zurücksetzen"},
       {"Version", "Ausführung"},
       {"Diagnostics", "Diagnose"},
       {"Compression", "Kompression"},
       {"Anlyz_crnt_waveform", "Aktuelle Wellenform analysieren"},
       {"Gnrt_trning_data", "Trainingsdaten generieren"},
       {"Timing-Belt", "Zahnriemen"},
       {"Fuel-Delivery-Pump", "Kraftstoff-Förderpumpe"},
       {"Common-Rail-Pressure", "Common-Rail-Druck"},
       {"Help", "Hilfe"},
       {"HelpLink", "Klicken Sie auf den Button um zur Hilfeseite zu gelangen"},
       {"Dvc_search", "Suche\ngeräte"},
       {"Save", "speichern"},
       {"Save warning", "Speicherwarnung"},
       {"No_dvc_available", "Es sind keine Geräte verfügbar ..."},
       {"AnalyzeData", "Daten\nanalysieren"},
       {"Crt_trng_data", "Erstellen\nSie\nTrainingsdaten"},
       {"Back", "Zurück"} };

static auto appLanguage = englishLan;  
static ImGui::FileBrowser fileBrowser;
static ImGui::FileBrowser directoryBrowser(ImGuiFileBrowserFlags_SelectDirectory);    
void generateTrainingData(bool&, 
                    const std::optional<OmniscopeSampler>&,
                    std::set<std::string>&);
void warning_popup(std::string_view, std::string_view); 

#endif