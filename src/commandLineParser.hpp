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
    bool searchDll = false;
};

void parseCommandLineArguments(int argc, char** argv, CommandLineOptions& options);

#endif
