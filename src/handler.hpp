// handler.hpp
#ifndef HANDLER_HEADER_HPP
#define HANDLER_HEADER_HPP

#include "mainWindow.hpp"
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "languages.hpp"
#include <implot.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>

// global variables
inline OmniscopeDeviceManager deviceManager{};
inline std::vector<std::shared_ptr<OmniscopeDevice>> devices;
inline std::map<Omniscope::Id, std::array<float, 3>> colorMap;
inline std::set<std::string> savedFileNames; // unique and ordered filenames
inline std::optional<OmniscopeSampler> sampler{};
inline std::map<Omniscope::Id, std::vector<std::pair<double, double>>>
    captureData;

void addPlots(
    const char *, mainWindow &, 
    std::function<void(double, std::string, ImAxis_, double, double)>);
void parseDeviceMetaData(Omniscope::MetaData,
                         std::shared_ptr<OmniscopeDevice> &);
void initDevices();
void devicesList(bool const &flagPaused);
void load_files(decltype(captureData) &, std::map<Omniscope::Id, std::string> &,
                bool &);
void set_config(mainWindow &);
void set_json(mainWindow &);
void set_inital_config(nlohmann::json &);
void setupSW(mainWindow &); 
void rstSettings();
void AddPlotFromFile(fs::path &); 
void addPlotFromFile(externData &); 

std::string trim(const std::string &); 

// Struct for loading data from a file :

struct LoadedFiles{
    public: 
        void LoadFromFile(fs::path &); 
        void printData(); // for debugging 
        void addToPlot(); 
        bool isScopeData;
        std::vector<std::string> units;
        std::vector<std::pair<double, double>> data;

    private:
        void parseData(const std::string &); 
        void parseUnits(const std::string &);
        
};



#endif
