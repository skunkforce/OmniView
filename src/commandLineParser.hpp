#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <string>
#include <set>
#include <CLI/CLI.hpp>

struct CommandLineOptions {
    std::string wsURI;
    std::vector<std::string> deviceIds;
    bool search = false;
    bool all = false;
};

void parseCommandLineArguments(int argc, char** argv, CommandLineOptions& options);

#endif
