#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <mutex>

class mainWindow{
    public: 
        nlohmann::json config; 
        const std::string configpath;
        nlohmann::json language;
        bool flagPaused{true}, open_generate_training_data{false}, open_analyze_menu{false}, open_settings{false}, LOADANALYSISDATA{false}; 

        bool development{false};
        std::once_flag configFlag;

        std::filesystem::path AnalyzedFilePath; 

        mainWindow(); 
        ~mainWindow(); 

        void setAnalyzePath(const std::filesystem::path &); 
};