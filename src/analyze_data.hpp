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
static Omniscope::Id selectedDevice{}; 

// declare functions and classes

void generate_analyze_menu(
    bool &,
    const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);

// Menustates
enum class State {
  NoDataSelected,
  FileDataSelected,
  FileDataLoaded,
  CurrentDataSelected,
  CurrentDataLoaded,
  DataWasSend,
  Reset
};

class AnalyzeStateManager {
public:
  AnalyzeStateManager() { currentState = State::NoDataSelected; }
  ~AnalyzeStateManager() = default;

  void setState(State state = State::NoDataSelected);
  State getState();

  void selectData(
      AnalyzeStateManager &, bool &,
      const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void loadData();

  void reset();

  void setMetaData();
  void clearMetaData();

  // extra functions:

  void selectCurrentDevice(const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> &);
  void selectFileData();
  void checkFileFormat();

  void writeAnalysisAnswerIntoFile();

private:
  State currentState;
  const std::map<Omniscope::Id, std::vector<std::pair<double, double>>> data;
};