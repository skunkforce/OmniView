#pragma once

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
#include "handler.hpp"
// declare functions and classes

fs::path generate_analyze_menu(
    bool &,bool &,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

// Menustates
enum class State {
  NODATA,
  FILEDATAWANTED,
  FILEDATASELECTED,
  FILEDATALOADED,
  CURRENTDATAWANTED,
  CURRENTDATASELECTED,
  CURRENTDATALOADED,
  DATAISSENDING, 
  DATAWASSEND,
  RESET
};

class AnalyzeStateManager {
public:
  AnalyzeStateManager() { 
    currentState = State::NODATA; 
    radioButtonCurrentData = false; 
    radioButtonFileData = false; 
    DeviceChecked = false;
    analyzeSaved = false; 
    selectedDeviceId = {}; 
    }
  ~AnalyzeStateManager() = default;

  void setState(State state = State::NODATA);
  State getState();

  void selectDataType(
      bool &,
      const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void selectData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  std::vector<double> loadData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

  void loadAndSendData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

  void generateAnalyzeAnswerPopUp(bool &);  

  void reset();

  void setMetaData();
  void clearMetaData();

  // extra functions:

  void selectCurrentDevice(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void whileSendingProcess(); 

  void writeAnalysisAnswerIntoFile();

  bool saveAsCSV(const std::string&);

  fs::path GetFilePath(); 


private:
  State currentState;
  const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> data;

  // extra variables for functions
  bool radioButtonCurrentData, radioButtonFileData, DeviceChecked, analyzeSaved;
  Omniscope::Id selectedDeviceId; 
  ImGui::FileBrowser AnalyzeFileBrowser; 
  std::string fileNameBuf; // path for a chosen file to load data from 
  std::future<std::string> future;
  nlohmann::ordered_json loadedDataJSON; 
  std::string apiResponse;
    fs::path outputFilePath; 
};