#include "mainWindow.hpp"

mainWindow::mainWindow() : configpath("config/config.json"){

    AnalyzedFilePath = ""; 
}

mainWindow::~mainWindow(){

}

void mainWindow::setAnalyzePath(const std::filesystem::path &AnalyzedFilePath){
 this->AnalyzedFilePath = AnalyzedFilePath; 
}