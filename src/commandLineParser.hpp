#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <string>
#include <set>
#include <CLI/CLI.hpp>

struct CommandLineOptions {
    std::string wsURI;
    std::string deviceId;
    bool search = false;
};

void parseCommandLineArguments(int argc, char** argv, CommandLineOptions& options);

#endif
