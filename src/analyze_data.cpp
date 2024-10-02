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
    ImGui::Text(appLanguage[Key::AnalyzeData]);
    stateManager.selectDataType(stateManager, open_analyze_menu, captureData);   // select if Data should be loaded from File or from a current measurement
    stateManager.selectData(stateManager, captureData); //select which device or which file the data should be loaded from 

  }

  // Close PopUp
  if (ImGui::Button(appLanguage[Key::Back])) {
    open_analyze_menu = false;
    ImGui::CloseCurrentPopup();
  }

  ImGui::EndPopup();
}

void AnalyzeStateManager::setState(State state) {
  this->currentState = state;
  std::cout << "Changed state" << std::endl;
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
    } else {
      stateManager.setState(State::CurrentDataSelected);
    }
  }

  // OR select a File from Browser
  if (ImGui::RadioButton(appLanguage[Key::Wv_from_file], radioButtonFileData)){
    radioButtonFileData = !radioButtonFileData; 
    radioButtonCurrentData = false; 
    stateManager.setState(State::FileDataSelected); 
  }

  info_popup(appLanguage[Key::WvForms_warning], appLanguage[Key::No_wave_made]);
}

void AnalyzeStateManager::selectData(AnalyzeStateManager &stateManager, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData) { 
      // Select Device from which to get the data, else select a file from which data can be loaded
    if(stateManager.getState() == State::CurrentDataSelected){
      if(ImGui::BeginCombo("##ComboDevice", "Devices & Waveforms Menu")){
        stateManager.selectCurrentDevice(captureData); 
      }
    }
    else if(stateManager.getState() == State::FileDataSelected){
      ImGui::InputTextWithHint("##inputLabel", appLanguage[Key::Csv_file],
                               &fileNameBuf, ImGuiInputTextFlags_ReadOnly);
      if(ImGui::Button(appLanguage[Key::Browse])) {
        AnalyzeFileBrowser.Open(); 
      }
       // Display FileBrowser each Frame, closes automatically when a file is selected 
       AnalyzeFileBrowser.SetTitle("Searching for .csv files");
       AnalyzeFileBrowser.Display();
    }
}

void AnalyzeStateManager::loadData(AnalyzeStateManager &stateManager, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData){};

void AnalyzeStateManager::reset() { std::cout << "Not used" << std::endl; }

void AnalyzeStateManager::setMetaData() {
  std::cout << "Not used" << std::endl;
}
void AnalyzeStateManager::clearMetaData() {
  std::cout << "Not used" << std::endl;
}

// extra functions:

void AnalyzeStateManager::selectCurrentDevice( const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &captureData) {
  for(const auto &[device, values] : captureData){
    if(ImGui::Checkbox(device.serial.c_str(), &DeviceChecked)){
      if(DeviceChecked){
      selectedDevice = device; 
      std::cout << "Device selected:" << device.serial.c_str() << std::endl; 
      }
      else {
      selectedDevice = {}; 
      std::cout << "Device Changed:" << device.serial.c_str() << std::endl; 
      }
    }
  }
}

void AnalyzeStateManager::selectFileData() {
  std::cout << "Not used" << std::endl;
}
void AnalyzeStateManager::checkFileFormat() {
  std::cout << "Not used" << std::endl;
}

void AnalyzeStateManager::writeAnalysisAnswerIntoFile() {
  std::cout << "Not used" << std::endl;
}
