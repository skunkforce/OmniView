#include "analyze_data.hpp"

void generate_analyze_menu( // generate the whole menu: Startpoint
    bool &open_analyze_menu,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData) {
  // Initialize AnalyzeStateManager

  static AnalyzeStateManager stateManager;
  // generate the PopUpMenu

  ImGui::OpenPopup(appLanguage[Key::AnalyzeData]);

  if (ImGui::BeginPopupModal(appLanguage[Key::AnalyzeData], nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    // Frame
    ImGui::Text(appLanguage[Key::AnalyzeData]);

    // select Data from File or current Data
    stateManager.selectData(stateManager, open_analyze_menu, captureData);
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

void AnalyzeStateManager::selectData(
    AnalyzeStateManager &stateManager, bool &open_analyze_menu,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
        &captureData) {

  // Select Current Wave
  if (ImGui::RadioButton(appLanguage[Key::Usr_curnt_wave],
                         radioButtonCurrentData)) {
    radioButtonCurrentData = !radioButtonCurrentData;
    // Check for current measurement, else error Popup
    if (!captureData.size()) {
      ImGui::OpenPopup(appLanguage[Key::WvForms_warning],
                       ImGuiPopupFlags_NoOpenOverExistingPopup);
      radioButtonCurrentData = false;
    } else {
      stateManager.setState(State::CurrentDataSelected);
      if(ImGui::BeginCombo("##ComboDevice", "Devices & Waveforms Menu")){ //choose the device from which the data is taken 
        stateManager.selectCurrentDevice(captureData); 
      }
    }
  }

  // Else select File from Browser

  // Default info popup for rendering

  info_popup(appLanguage[Key::WvForms_warning], appLanguage[Key::No_wave_made]);
}
void AnalyzeStateManager::loadData() { std::cout << "Not used" << std::endl; }

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
