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
// extra boolean references for the RadioButtons

static bool radioButtonCurrentData{false}, radioButtonFileData{false}, DeviceChecked{false};
static Omniscope::Id selectedDeviceId{}; 
static ImGui::FileBrowser AnalyzeFileBrowser; 
static std::string fileNameBuf; // path for a chosen file to load data from 

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
  AnalyzeStateManager() { currentState = State::NODATA; }
  ~AnalyzeStateManager() = default;

  void setState(State state = State::NODATA);
  State getState();

  void selectDataType(
      AnalyzeStateManager &, bool &,
      const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void selectData(AnalyzeStateManager &, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void loadData(AnalyzeStateManager &, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

  void loadAndSendData(AnalyzeStateManager &, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &); 

  void reset(AnalyzeStateManager &);

  void setMetaData();
  void clearMetaData();

  // extra functions:

  void selectCurrentDevice(AnalyzeStateManager &, const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

  void writeAnalysisAnswerIntoFile();

private:
  State currentState;
  const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> data;
};