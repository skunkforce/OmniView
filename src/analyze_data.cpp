#include "analyze_data.hpp"

fs::path generate_analyze_menu( // generate the whole menu: Startpoint
    bool &open_analyze_menu,bool &LOADANALYSISDATA,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData) {

  static AnalyzeStateManager stateManager; // Initialize AnalyzeStateManager

  // generate the PopUpMenu
  ImGui::OpenPopup(appLanguage[Key::AnalyzeData]);

  if (ImGui::BeginPopupModal(appLanguage[Key::AnalyzeData], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    // Frame
    if(stateManager.getState() == State::RESET){
      stateManager.reset(); 
    }
    ImGui::Text(appLanguage[Key::AnalyzeData]);
    stateManager.selectDataType(open_analyze_menu, captureData);   // select if Data should be loaded from File or from a current measurement
    stateManager.selectData(captureData); //select which device or which file the data should be loaded from 
    stateManager.setMetaData(); // here there could be an array that is requested before from the API that loads the correct metadata files, currently no MetaData needed
  
  // Close PopUp
  if (ImGui::Button(appLanguage[Key::Back])) {
    stateManager.reset(); 
    open_analyze_menu = false;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine(); 
  // Send Data 
  if(ImGui::Button(appLanguage[Key::Send])){
    stateManager.loadAndSendData(captureData); // One function because of the loading bar 
  }
  if(stateManager.getState() == State::DATAISSENDING){ // waiting for API response
    stateManager.whileSendingProcess(); 
  } 
  if(stateManager.getState() == State::DATAWASSEND){
     ImGui::OpenPopup(appLanguage[Key::Data_upload],
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
     stateManager.generateAnalyzeAnswerPopUp(LOADANALYSISDATA); 
  }
  }
  ImGui::EndPopup();
  fs::path FilePath = stateManager.GetFilePath(); 
  return FilePath; 
}

void AnalyzeStateManager::setState(State state) {
  this->currentState = state;
}

State AnalyzeStateManager::getState() { return this->currentState; }

void AnalyzeStateManager::selectDataType(
    bool &open_analyze_menu,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData) {

  // Select Current Wave
  if (ImGui::RadioButton(appLanguage[Key::Usr_curnt_wave],
                         radioButtonCurrentData)) {
    radioButtonCurrentData = !radioButtonCurrentData;
    radioButtonFileData = false; 
    if (!captureData.size()) { // Check for current measurement, else error Popup
      ImGui::OpenPopup(appLanguage[Key::WvForms_warning],
                       ImGuiPopupFlags_NoOpenOverExistingPopup);
      radioButtonCurrentData = false;
      currentState = State::NODATA; 
    } else if (radioButtonCurrentData)
    {
      currentState = State::CURRENTDATAWANTED;
    }
    else currentState = State::NODATA; 
  }

  ImGui::SeparatorText(" ");

  // OR select a File from Browser
  if (ImGui::RadioButton(appLanguage[Key::Wv_from_file], radioButtonFileData)){
    radioButtonFileData = !radioButtonFileData; 
    radioButtonCurrentData = false; 
    if (radioButtonFileData){
    currentState = State::FILEDATAWANTED;}
    else currentState = State::NODATA; 
  }
  ImGui::SeparatorText(" "); 

  info_popup(appLanguage[Key::WvForms_warning], appLanguage[Key::No_wave_made]);
}

void AnalyzeStateManager::selectData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData) { 
      // Select Device from which to get the data, else select a file from which data can be loaded
    if(currentState == State::CURRENTDATAWANTED || currentState == State::CURRENTDATASELECTED){
      if(ImGui::BeginCombo("##ComboDevice", "Devices & Waveforms Menu")){
        this->selectCurrentDevice(captureData);
        ImGui::EndCombo();  
      }
    }
    else if((currentState == State::FILEDATAWANTED) || (currentState == State::FILEDATASELECTED)){
      ImGui::InputTextWithHint("##inputLabel", appLanguage[Key::Csv_file],
                               &fileNameBuf, ImGuiInputTextFlags_ReadOnly);
      ImGui::SameLine(); 
      if(ImGui::Button(appLanguage[Key::Browse])) {
        AnalyzeFileBrowser.Open(); 
      }
       // Display FileBrowser each Frame, closes automatically when a file is selected 
       AnalyzeFileBrowser.SetTitle("Searching for .csv files");
       AnalyzeFileBrowser.Display();

       if(AnalyzeFileBrowser.HasSelected()){
        if(fs::path(AnalyzeFileBrowser.GetSelected().string()).extension() == ".csv"){
          fileNameBuf = AnalyzeFileBrowser.GetSelected().string(); 
          AnalyzeFileBrowser.ClearSelected();
          currentState = State::FILEDATASELECTED; 
        }
        else {
          ImGui::OpenPopup(appLanguage[Key::Wrong_file_warning],
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
          AnalyzeFileBrowser.ClearSelected();
          currentState = State::RESET; 
        }
       }
    }
    // possible error popup 
       info_popup(appLanguage[Key::Wrong_file_warning],
               appLanguage[Key::Wrong_file_type]);
}

void AnalyzeStateManager::loadAndSendData( const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData){
  // disableAllOtherFields(); 

  std::vector<double> loadedData = this->loadData(captureData); // loading the data 

  if(currentState != State::DATAISSENDING){ // start asynch task to load data 
    loadedDataJSON["meta"] = {"metadaten"}; 
    loadedDataJSON["data"] = {{"sampling_rate", 100000}, {"y_values", loadedData}}; 

    future = std::async(std::launch::async, [&] {
          // take temp object returned from dump() and send it to sendData
          std::string result = sendData(loadedDataJSON.dump());
          return result;
        });
    currentState = State::DATAISSENDING; 
  }
  //sendData() from sendData.hpp on the Json.dump() output needs enableAllOtherFields(); after api_message is received is finished
}

std::vector<double> AnalyzeStateManager::loadData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData){
 std::vector<double> y_values; 
 if(currentState == State::CURRENTDATASELECTED){
    auto selectedDevice(captureData.find(selectedDeviceId)); 
    if(selectedDevice != captureData.end()){
      y_values.resize(selectedDevice->second.size()); 
      for(int i = 0; i < selectedDevice->second.size(); ++i){
        y_values[i] = selectedDevice->second[i].second; 
      }
    }
    currentState = State::CURRENTDATALOADED; 
 }
 else if (currentState == State::FILEDATASELECTED){
    fs::path filePath = fs::path(fileNameBuf); 
    std::ifstream selectedFile(filePath, std::ios::binary); 

    if(!selectedFile.is_open()){
       fmt::println("Failed to open file {}", filePath);
    }
    else {
      // pop first line 
        std::string first_line;
        std::getline(selectedFile, first_line);
      // read the values from the file 
      double value{};
        while (selectedFile >> value) {  
            y_values.emplace_back(value);
            static constexpr size_t bigNumber{10'000'000};
            selectedFile.ignore(bigNumber, '\n');  
        }

        if (!y_values.empty()) {
            y_values.pop_back();  
        }
    y_values.pop_back(); // pop last element
    currentState = State::FILEDATALOADED; 
   }
 }
 return y_values; 
};

void AnalyzeStateManager::reset() { 
  radioButtonCurrentData = false; 
  radioButtonFileData = false; 
  selectedDeviceId = {}; 
  currentState = State::NODATA; 

}

void AnalyzeStateManager::setMetaData() { 
  // will be implemented when there are analysis where Metadata is needed
}
void AnalyzeStateManager::clearMetaData() {
  // will be implemented when there are analysis where Metadata is needed
}

// extra functions:

void AnalyzeStateManager::selectCurrentDevice(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData) {
  for(const auto &[deviceId, values] : captureData){
    if(ImGui::Checkbox(deviceId.serial.c_str(), &DeviceChecked)){
      if(DeviceChecked){
      selectedDeviceId = deviceId; 
      currentState = State::CURRENTDATASELECTED;
      }
      else {
      selectedDeviceId = {}; 
      currentState = State::CURRENTDATAWANTED;
      }
    }
  }
}

void AnalyzeStateManager::whileSendingProcess(){
  auto status = future.wait_for(std::chrono::milliseconds(10));
    if(status == std::future_status::ready){
      if(future.valid()){
        apiResponse = future.get(); 
      }
      loadedDataJSON.clear(); 
      currentState = State::DATAWASSEND;  
    }
    else {
      ImGui::SameLine();
        ImGui::Text(" sending ... %c",
                    "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    }
}

void AnalyzeStateManager::generateAnalyzeAnswerPopUp(bool &LOADANALYSISDATA){
  if(ImGui::BeginPopupModal(appLanguage[Key::Data_upload]), nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize){
    ImGui::Text(appLanguage[Key::Analyse_Answer_Text]); 
    ImGui::SeparatorText(" ");
    ImGui::Text(apiResponse == "empty" ? appLanguage[Key::Upload_failure]
                                      : appLanguage[Key::Upload_success]);
                                      ImGui::SeparatorText(" ");
    if(!analyzeSaved){
      this->writeAnalysisAnswerIntoFile();
    } 
    if(ImGui::Button(appLanguage[Key::SeeAnalyzeResults])){
      std::cout << "See results" << std::endl; 
      LOADANALYSISDATA = true; 
      reset(); 
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine(); 
    if(ImGui::Button(appLanguage[Key::Back])) {
    reset(); 
    ImGui::CloseCurrentPopup();
    }
  ImGui::EndPopup(); 
  }
}

void AnalyzeStateManager::writeAnalysisAnswerIntoFile() {
   if (apiResponse != "empty") {
      fs::path complete_path = fs::current_path() / "analyze";
      if (!fs::exists(complete_path))
        fs::create_directories(complete_path);

      outputFilePath = complete_path / ("Analysis_" + fs::path(fileNameBuf).filename().string());

      std::ofstream writefile(outputFilePath, std::ios::trunc);
      if (!writefile.is_open()) {
        writefile.clear();
        fmt::println("Could not create {} for writing!", outputFilePath.string());
      } else {
        fmt::println("Start saving {}.", outputFilePath.string());
        bool test = saveAsCSV(apiResponse); 
        std::cout << "test:" << test << std::endl; 
        /*writefile << apiResponse;
        writefile.flush();
        writefile.close();*/
        fmt::println("Finished saving CSV file.");
      }
      analyzeSaved = true; 
    } 
}

bool AnalyzeStateManager::saveAsCSV(const std::string& apiResponse) {
    try {
        // Parse den API-Response-String in ein JSON-Objekt
        nlohmann::json jsonData = nlohmann::json::parse(apiResponse);

        // Öffne die Ausgabedatei im Schreibmodus
        std::ofstream outFile(outputFilePath);
        if (!outFile.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei zum Schreiben." << std::endl;
            return false;
        }

        // Schreibe die Metadatenzeile
        outFile << "FFT\n";  // Erste Zeile mit dem Inhalt "FFT"

        // Schreibe die Einheitenzeile
        outFile << "frq,amp\n";  // Zweite Zeile mit den Einheiten "frq" und "amp"

        // Schleife über die "fft"-Daten im JSON-Array und schreibe sie in die Datei
        for (const auto& item : jsonData["fft"]) {
            double frequency = item["frequency"];
            double amplitude = item["amplitude"];

            // Schreibe die Frequenz und Amplitude, getrennt durch ein Komma
            outFile << frequency << "," << amplitude << "\n";
        }

        // Schließe die Datei
        outFile.close();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Fehler beim Verarbeiten der JSON-Daten: " << e.what() << std::endl;
        return false;
    }
}

fs::path AnalyzeStateManager::GetFilePath(){
  return this->outputFilePath; 
}