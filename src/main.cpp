#include <cmake_git_version/version.hpp>
#include "websockethandler.hpp"

int main() {
    
  bool flagPaused{true};
  std::set<std::string> selected_serials;

  WebSocketHandler wsHandler("ws://localhost:8080/");

  // Start console handler thread
  std::thread consoleThread(consoleHandler, std::ref(flagPaused), std::ref(selected_serials));

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

  consoleThread.join();
  webSocketThread.detach();

  return 0;
}
