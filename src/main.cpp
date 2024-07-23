#include "style.hpp"
#include <cmake_git_version/version.hpp>
#include "jasonhandler.hpp"
#include "websockethandler.hpp"

int main() {
    
  const std::string configpath = "config/config.json";
  set_config(configpath);
  nlohmann::json config = load_json_file(configpath);
  set_json(config);
  bool flagPaused{true};
  bool flagInitState{true};
  std::set<std::string> selected_serials;

  WebSocketHandler wsHandler("ws://localhost:8080/");

  // Start console handler thread
  std::thread consoleThread(consoleHandler, std::ref(flagInitState), std::ref(config), std::ref(flagPaused), std::ref(selected_serials));

  // WebSocket handler thread
  std::thread webSocketThread([&]() {
    while (true) {
        if (sampler.has_value() && !flagPaused) {
        sampler->copyOut(captureData);
        wsHandler.send(captureData, selected_serials);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  });

  /*
  // main logic without GUI
  if (flagInitState) {
  set_inital_config(config);
  flagInitState = false;
  }
  */

  consoleThread.join();
  webSocketThread.detach();

  return 0;
}
