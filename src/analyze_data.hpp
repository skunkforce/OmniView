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

void generate_analyze_menu(); 

enum class State {
    NoDataSelected, 
    FileDataSelected, 
    FileDataLoaded, 
    CurrentDataSelected, 
    CurrentDataLoaded,
    DataWasSend,
    Reset
}; 

class StateManager{
public: 
    StateManager(); 

    
}