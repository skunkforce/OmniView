#include "analyze_data.hpp"

void generate_analyze_menu( // generate the whole menu: Startpoint
    bool &open_analyze_menu,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData) {

  static AnalyzeStateManager stateManager; // Initialize AnalyzeStateManager

  // generate the PopUpMenu
  ImGui::OpenPopup(appLanguage[Key::AnalyzeData]);

  if (ImGui::BeginPopupModal(appLanguage[Key::AnalyzeData], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    // Frame
    if(stateManager.getState() == State::RESET){
      stateManager.reset(stateManager); 
    }
    ImGui::Text(appLanguage[Key::AnalyzeData]);
    stateManager.selectDataType(stateManager, open_analyze_menu, captureData);   // select if Data should be loaded from File or from a current measurement
    stateManager.selectData(stateManager, captureData); //select which device or which file the data should be loaded from 
    stateManager.setMetaData(); // here there could be an array that is requested before from the API that loads the correct metadata files, currently no MetaData needed

  // Send Data 
  if(ImGui::Button(appLanguage[Key::Send])){
    stateManager.loadAndSendData(stateManager, captureData); // One function because of the loading bar 
  }
  // Close PopUp
  if (ImGui::Button(appLanguage[Key::Back])) {
    stateManager.reset(stateManager); 
    open_analyze_menu = false;
    ImGui::CloseCurrentPopup();
  }
  }

  ImGui::EndPopup();
}

void AnalyzeStateManager::setState(State state) {
  this->currentState = state;
}

State AnalyzeStateManager::getState() { return this->currentState; }

void AnalyzeStateManager::selectDataType(
    AnalyzeStateManager &stateManager, bool &open_analyze_menu,
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
      stateManager.setState(State::NODATA); 
    } else if (radioButtonCurrentData)
    {
      stateManager.setState(State::CURRENTDATAWANTED);
    }
    else stateManager.setState(State::NODATA); 
  }

  // OR select a File from Browser
  if (ImGui::RadioButton(appLanguage[Key::Wv_from_file], radioButtonFileData)){
    radioButtonFileData = !radioButtonFileData; 
    radioButtonCurrentData = false; 
    if (radioButtonFileData){
    stateManager.setState(State::FILEDATAWANTED);}
    else stateManager.setState(State::NODATA); 
  }

  info_popup(appLanguage[Key::WvForms_warning], appLanguage[Key::No_wave_made]);
}

void AnalyzeStateManager::selectData(AnalyzeStateManager &stateManager, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData) { 
      // Select Device from which to get the data, else select a file from which data can be loaded
    if(stateManager.getState() == State::CURRENTDATAWANTED){
      if(ImGui::BeginCombo("##ComboDevice", "Devices & Waveforms Menu")){
        stateManager.selectCurrentDevice(stateManager, captureData);
        ImGui::EndCombo();  
      }
    }
    else if(stateManager.getState() == State::FILEDATAWANTED){
      ImGui::InputTextWithHint("##inputLabel", appLanguage[Key::Csv_file],
                               &fileNameBuf, ImGuiInputTextFlags_ReadOnly);
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
          stateManager.setState(State::FILEDATASELECTED); 
        }
        else {
          ImGui::OpenPopup(appLanguage[Key::Wrong_file_warning],
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
          AnalyzeFileBrowser.ClearSelected();
          stateManager.setState(State::RESET); 
        }
       }
    }
    // possible error popup 
       info_popup(appLanguage[Key::Wrong_file_warning],
               appLanguage[Key::Wrong_file_type]);
}

void AnalyzeStateManager::loadAndSendData(AnalyzeStateManager &stateManager, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData){
  loadData(stateManager, captureData); // The data will be loaded into a json file
  //sendData() from sendData.hpp on the Json.dump() output 
}

void AnalyzeStateManager::loadData(AnalyzeStateManager &stateManager, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData){
 if(stateManager.getState() == State::CURRENTDATASELECTED){
        auto selectedDevice(captureData.find(selectedDeviceId)); 
        if(selectedDevice != captureData.end()){
          //TODO Load Data 
        }

 }
 else if (stateManager.getState() == State::FILEDATASELECTED){
      
 }

};

void AnalyzeStateManager::reset(AnalyzeStateManager &stateManager) { 
  radioButtonCurrentData = false; 
  radioButtonFileData = false; 
  stateManager.setState(State::NODATA); 

}

void AnalyzeStateManager::setMetaData() { 
  // will be implemented when there are analysis where Metadata is needed
}
void AnalyzeStateManager::clearMetaData() {
  // will be implemented when there are analysis where Metadata is needed
}

// extra functions:

void AnalyzeStateManager::selectCurrentDevice( AnalyzeStateManager &stateManager, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData) {
  for(const auto &[deviceId, values] : captureData){
    if(ImGui::Checkbox(deviceId.serial.c_str(), &DeviceChecked)){
      if(DeviceChecked){
      selectedDeviceId = deviceId; 
      stateManager.setState(State::CURRENTDATASELECTED);
      std::cout << "Device selected:" << deviceId.serial.c_str() << std::endl;
      }
      else {
      selectedDeviceId = {}; 
      stateManager.setState(State::CURRENTDATAWANTED);
      std::cout << "Device Changed:" << deviceId.serial.c_str() << std::endl; 
      }
    }
  }
}

void AnalyzeStateManager::writeAnalysisAnswerIntoFile() {
  std::cout << "Not used" << std::endl;
}
