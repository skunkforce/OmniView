#include "analyze_data.hpp"

void generate_analyze_menu(bool &open_analyze_menu){

    // Initialize AnalyzeStateManager 

    static AnalyzeStateManager stateManager(State::NoDataSelected); 
    
    // generate the PopUpMenu 

    ImGui::OpenPopup(appLanguage[Key::AnalyzeData]); 

    if(ImGui::BeginPopupModal(appLanguage[Key::AnalyzeData], nullptr, ImGuiWindowFlags_AlwaysAutoResize)){

        ImGui::Text(appLanguage[Key::AnalyzeData]);  

    }


    //Close PopUp
    if (ImGui::Button(appLanguage[Key::Back])) {
      open_analyze_menu = false;
      ImGui::CloseCurrentPopup();
    }

     ImGui::EndPopup();


}


    void AnalyzeStateManager::SetState(State state){
        std::cout << "Not used" << std::endl; 
    }
    State AnalyzeStateManager::getState(){
           std::cout << "Not used" << std::endl; 
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
