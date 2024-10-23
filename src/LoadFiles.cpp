#include "loadFiles.hpp"

namespace fs = std::filesystem;

externData::externData(const std::filesystem::path& path) : filepath(path) {};
externData::~externData(){}; 
 
void generateLoadFilesMenu(std::vector<std::filesystem::path> &externDataFilePaths, std::vector<externData> &externDatas, bool &close){
    ImGui::OpenPopup(appLanguage[Key::Load_file_data]);

    if (ImGui::BeginPopupModal(appLanguage[Key::Load_file_data], nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetItemDefaultFocus();
        static ImGui::FileBrowser fileBrowser;
        static std::vector<std::string> pathArr;

        if (pathArr.empty())
            pathArr.emplace_back("");

        for (auto &path : pathArr) {
            ImGui::PushID(&path); // unique IDs
            ImGui::InputTextWithHint("##", "Path", &path);
            ImGui::SameLine();

            if (ImGui::Button("Browse")) {
                fileBrowser.Open();
            }

            fileBrowser.Display();
            
            if (fileBrowser.HasSelected()) {
                path = fileBrowser.GetSelected().string(); 
                fileBrowser.ClearSelected();
            }

            // Validierung des Dateipfads
            if (!path.empty()) {
                if (fs::path(path).extension() != ".csv") {  // only accept .csv data
                    ImGui::OpenPopup("Wrong File Type");
                    path.clear();
                } else {
                    if (std::find(externDataFilePaths.begin(), externDataFilePaths.end(), path) == externDataFilePaths.end()) {
                    // Füge den Pfad nur hinzu, wenn er nicht bereits vorhanden ist
                    externDataFilePaths.emplace_back(path);
                    } else {
                    std::cout << "Path " << path << " was already chosen.\n";
                    }
                }
            }
            ImGui::PopID();
        }
        
        if (ImGui::Button(" + ")) {
            pathArr.emplace_back("");
        }

        if (ImGui::Button("Back")) {
            pathArr.clear();
            externDataFilePaths.clear();
            close = false; 
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Load Files 
        if (ImGui::Button("Load Files")) {
            loadMultipleExternFiles(externDataFilePaths, externDatas);
            pathArr.clear(); 
            close = false; 
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void loadMultipleExternFiles(std::vector<std::filesystem::path> &paths, std::vector<externData> &dataObjects){

     for (const auto& path : paths) {
        // new data object for new path
        externData data(path);
    
        data.loadDataFromFile();
        
        dataObjects.push_back(data); // Push_back because the dataObjects vector should be dynamically and is changed in other functions as well
        
        std::cout << "Data form file " << path.string() << " was loaded.\n";
    }
}

void externData::loadDataFromFile() {

    if (isLoaded) {
        std::cout << "File " << filepath.string() << " was already loaded.\n";
        return; 
    }

    // Öffne die Datei
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: File cant be opened " + filepath.string());
    }

    std::string line;

    // Check for old data format with Omniscope 
    if (std::getline(file, line)) {
        if (line.find("Omniscope") != std::string::npos) {
            isOmnAIScope = true;
            std::cout << "Old OmnAIScope file detected\n";
            sampling_rate = 100000; // sampling rate of the OmnAIScope
        }
    }

    // Try reading units from the second line, else set standard units
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string unit;
        units.clear(); 

        while (iss >> unit) {
            units.push_back(unit);
        }
        if (units.size() < 2) {
            std::cout << "No units provided, standard units (V) and (s) are set" << std::endl;
            units = {"s", "V"};  // Standardwerte setzen (s für Zeit, V für Spannung)
        }
    } else {
        // Falls die Zeile mit den Einheiten fehlt, setze Standardwerte
        std::cout << "No Units provided. Standard Units (V) and (s) are set" << std::endl;
        units = {"s", "V"};  // Standardwerte setzen
    }

    double x = 0.0, y = 0.0;
    int index = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);

        if (isOmnAIScope) {
            // OmniScope-Modus: Nur Y-Werte vorhanden, X-Werte basierend auf Sampling-Rate berechnen
            x = index / sampling_rate;  // X-Wert berechnen
            if (!(iss >> y)) {
                throw std::runtime_error("Error: Y-Values could not be read correctly");
            }
        } else {
            // Normaler Modus: X- und Y-Werte aus der Datei lesen
            if (!(iss >> x >> y)) {
                throw std::runtime_error("Error: Y and X-Values could not be read correctly");
            }
        }

        // Speichere die gelesenen Werte
        xValues.push_back(x);
        yValues.push_back(y);

        // Erhöhe den Index für die nächste Berechnung (Sampling-Rate) bei OmniScope
        index++;
    }

    // Schließe die Datei
    file.close();

    // Bestätige, dass die Daten erfolgreich geladen wurden
    std::cout << "Data from file " << filepath.string() << " was loaded sucessfully\n";
}

