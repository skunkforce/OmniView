#include "commandLineParser.hpp"

void parseCommandLineArguments(int argc, char** argv, CommandLineOptions& options) {
    CLI::App app{"OmniView"};

    app.add_option("-w, --wsuri", options.wsURI, "WebSocket URI");
    app.add_option("-d, --device", options.deviceIds, "Omniscope Device ID")->expected(-1);
    app.add_flag("-s, --searchdevice", options.searchDevice, "Search for devices");
    app.add_flag("-a, --all", options.all, "Use all connected devices");
    app.add_option("-p, --path", options.dllSearchPath, "Path to search for DLLs");
    app.add_option("-n, --dllName", options.dllName, "Name of the DLL to load");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        std::exit(app.exit(e));
    }
}
