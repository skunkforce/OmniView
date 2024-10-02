#include <fstream>
#include <regex>
#include <imgui.h>
#include <filesystem>
#include <sstream>
#include <limits>
#include <future>
#include <iostream>
#include <cstring>
#include "languages.hpp"
#include "popups.hpp"
#include "sendData.hpp"
#include "../imgui-stdlib/imgui_stdlib.h"
#include "jasonhandler.hpp"

void generate_analyze_menu(bool &); 

enum class State {
    NoDataSelected, 
    FileDataSelected, 
    FileDataLoaded, 
    CurrentDataSelected, 
    CurrentDataLoaded,
    DataWasSend,
    Reset
}; 

class AnalyzeStateManager{
public: 
    AnalyzeStateManager(State state = State::NoDataSelected){
        
    }
    ~AnalyzeStateManager() = default; 

    void SetState(State state = State::NoDataSelected); 
    State getState();  

    void selectData(); 
    void loadData(); 

    void reset(); 

    void setMetaData(); 
    void clearMetaData(); 

    // extra functions: 

    void selectCurrentDevice(); 
    void selectFileData(); 
    void checkFileFormat(); 

    void writeAnalysisAnswerIntoFile(); 


private: 

    State currentState; 
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> data; 
};