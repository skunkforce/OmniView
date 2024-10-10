#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <string>
#include <set>
#include <CLI/CLI.hpp>

struct CommandLineOptions {
    std::string wsURI;
    std::vector<std::string> deviceIds;
    bool searchDevice = false;
    bool all = false;
    std::string dllSearchPath;
    std::string dllName;
};

void parseCommandLineArguments(int argc, char** argv, CommandLineOptions& options);

#endif
