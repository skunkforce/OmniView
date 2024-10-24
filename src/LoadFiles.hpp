#ifndef LOADFILES_HPP
#define LOADFILES_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <map>
#include <filesystem>
#include <imgui.h>
#include <implot.h>
#include "languages.hpp"
#include "../imgui-stdlib/imgui_stdlib.h"
#include "../imgui-filebrowser/imfilebrowser.h"


class externData{

    public: 
        std::map<std::string, std::string> metaData;
        std::vector<std::string> units;
        std::vector<double> xValues; 
        std::vector<double> yValues; 
        std::filesystem::path filepath; 
        bool isOmnAIScope, loadChecked{false}, isLoaded{false}, showData{false}; 
        double sampling_rate = 0.0; 

        externData(const std::filesystem::path&);
        ~externData(); 

        void loadDataFromFile();

        //void ShowPlot(); // alternative to AddPlotsFromFile for various .csv formats 

};

void generateLoadFilesMenu(std::vector<std::filesystem::path> &, std::vector<externData> &, bool &); 

void loadMultipleExternFiles(std::vector<std::filesystem::path> &, std::vector<externData> &);

void filesList(std::vector<externData> &);  // generates, shows and functionality of the filesList in the DeviceRegion

void addPlotFromFile(externData &); 
void addPlotFromFFTFile(externData &);  

#endif