#include "analyze_data.hpp"

void generate_analyze_menu(bool &open_analyze_menu){

    // Initialize AnalyzeStateManager 

    static AnalyzeStateManager stateManager; 
    
    // generate the PopUpMenu 

    ImGui::OpenPopup(appLanguage[Key::AnalyzeData]); 

    if(ImGui::BeginPopupModal(appLanguage[Key::AnalyzeData], nullptr, ImGuiWindowFlags_AlwaysAutoResize)){

        ImGui::Text(appLanguage[Key::AnalyzeData]);  

        if(ImGui::RadioButton(appLanguage[Key::Usr_curnt_wave], radioButtonCurrentData)){
            radioButtonCurrentData = !radioButtonCurrentData; 
            stateManager.setState(State::CurrentDataSelected); 
        }

    }


    //Close PopUp
    if (ImGui::Button(appLanguage[Key::Back])) {
      open_analyze_menu = false;
      ImGui::CloseCurrentPopup();
    }

     ImGui::EndPopup();


}


    void AnalyzeStateManager::setState(State state){

        this->currentState= state ; 
        std::cout << "Changed state" << std::endl; 
    }
    State AnalyzeStateManager::getState(){
           return this->currentState; 
    }  

    void AnalyzeStateManager::selectData(){
           std::cout << "Not used" << std::endl; 
    }
    void AnalyzeStateManager::loadData(){
           std::cout << "Not used" << std::endl; 
    }

    void AnalyzeStateManager::reset(){
           std::cout << "Not used" << std::endl; 
    }

    void AnalyzeStateManager::setMetaData(){
           std::cout << "Not used" << std::endl; 
    }
    void AnalyzeStateManager::clearMetaData(){
           std::cout << "Not used" << std::endl; 
    }

    // extra functions: 

    void AnalyzeStateManager::selectCurrentDevice(){
           std::cout << "Not used" << std::endl; 
    }
    void AnalyzeStateManager::selectFileData(){
           std::cout << "Not used" << std::endl; 
    }
    void AnalyzeStateManager::checkFileFormat(){
           std::cout << "Not used" << std::endl; 
    }

    void AnalyzeStateManager::writeAnalysisAnswerIntoFile(){
           std::cout << "Not used" << std::endl; 
    }
