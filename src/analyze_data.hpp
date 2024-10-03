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
// declare functions and classes

void generate_analyze_menu(
    bool &,
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
    selectedDeviceId = {}; 
    }
  ~AnalyzeStateManager() = default;

  void setState(State state = State::NODATA);
  State getState();

  void selectDataType(
      bool &,
      const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void selectData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void loadData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

  void loadAndSendData(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &); 

  void reset();

  void setMetaData();
  void clearMetaData();

  // extra functions:

  void selectCurrentDevice(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

  void writeAnalysisAnswerIntoFile();

private:
  State currentState;
  const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> data;

  // extra variables for functions
  bool radioButtonCurrentData, radioButtonFileData, DeviceChecked;
  Omniscope::Id selectedDeviceId; 
  ImGui::FileBrowser AnalyzeFileBrowser; 
  std::string fileNameBuf; // path for a chosen file to load data from 
};