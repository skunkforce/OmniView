#include "loadingFiles.hpp"

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

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: File cant be opened " + filepath.string());
    }

   std::string line;

    bool firstLineIsData = false;  // Flag, um zu überprüfen, ob die erste Zeile Werte enthält
    bool secondLineIsData = false; // Flag, ob die zweite Zeile Werte enthält
    units.clear();

    // Check for OmniScope in the first line or if it's data
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        double potential_x, potential_y;
        if (line.find("Omniscope") != std::string::npos) {
            isOmnAIScope = true;
            std::cout << "Old OmnAIScope file detected\n";
            sampling_rate = 100000;  // Set the sampling rate for the OmniScope format
        } else if (iss >> potential_x >> potential_y) {
            // If the first line contains two doubles, it is treated as data
            firstLineIsData = true;
            xValues.push_back(potential_x);
            yValues.push_back(potential_y);
            if (units.size() < 2) {
            std::cout << "No units provided, standard units (s) and (V) are set" << std::endl;
            units = {"s", "V"};  // Set standard units
            }
        }
    }

    // Check for units in the second line or if it's data
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        double potential_x, potential_y;
        std::string unit;

        if (iss >> unit && !std::isdigit(unit[0])) {
            // If the second line contains units (strings), add them to the units vector
            units.push_back(unit);
            while (iss >> unit) {
                units.push_back(unit);
            }
        } else if (iss >> potential_x >> potential_y) {
            // If the second line contains two doubles, treat it as data
            secondLineIsData = true;
            xValues.push_back(potential_x);
            yValues.push_back(potential_y);
        }

        // If no units were found, set the standard units
        if (units.size() < 2) {
            std::cout << "No units provided, standard units (s) and (V) are set" << std::endl;
            units = {"s", "V"};  // Set standard units
        }
    }

    double x = 0.0, y = 0.0;
    int index = firstLineIsData + secondLineIsData;  // Start the index based on pre-read data

    // Continue reading the rest of the file
    while (std::getline(file, line)) {
        std::istringstream iss(line);
         char comma;

        if (isOmnAIScope) {
            // OmniScope format: Only Y-values are present, X is calculated
            x = index / sampling_rate;
            if (!(iss >> y)) {
                throw std::runtime_error("Error: Y-Values could not be read correctly");
            }
        } else {
            // Normal format: Read X and Y values
             if (!(iss >> x >> comma >> y) || comma != ',') {
                throw std::runtime_error("Error: X and Y-Values could not be read correctly");
            }
        }

        xValues.push_back(x);
        yValues.push_back(y);

        index++;
    }

    // Schließe die Datei
    file.close();

    // Bestätige, dass die Daten erfolgreich geladen wurden
    std::cout << "Data from file " << filepath.string() << " was loaded sucessfully\n";
}


void filesList(std::vector<externData> &dataObjects) { // Show list of files in Devices Region
    ImGui::BeginGroup();
    
    if (!dataObjects.empty()) {
        for (auto it = dataObjects.begin(); it != dataObjects.end(); ) {
            externData& obj = *it; 

            if (ImGui::Checkbox("##", &obj.loadChecked)) {
                if (obj.loadChecked) {
                    obj.showData = true; 
                    //TODO:: Filter if FFT and Plot a Histogram
                    
                } else {
                    obj.showData = false; 
                }
            }
            
            ImGui::SameLine();
            ImGui::TextUnformatted(obj.filepath.string().c_str());
            ImGui::SameLine();

            //Deleting Loaded Data
            if (ImGui::Button(appLanguage[Key::Reset])) {
                it = dataObjects.erase(it);  
            } else {
                ++it; 
            }
        }
    }
    
    ImGui::EndGroup();
}


void addPlotFromFFTFile(externData &dataObj) {
   
}

