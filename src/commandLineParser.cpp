#include "commandLineParser.hpp"

void parseCommandLineArguments(int argc, char** argv, CommandLineOptions& options) {
    CLI::App app{"OmniView"};

    app.add_option("-w, --wsuri", options.wsURI, "WebSocket URI");
    app.add_option("-d, --device", options.deviceId, "Omniscope Device ID");
    app.add_flag("-s, --search", options.search, "Search for devices");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        std::exit(app.exit(e));
    }
}
