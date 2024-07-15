#include "crow.h"
#include "OmniscopeSampler.hpp"
#include <thread>
#include <atomic>
#include <sstream>
#include <iostream>

// Globale Variable für WebSocket-Verbindungen
std::vector<crow::websocket::connection*> connections;
std::mutex connections_mutex;

void broadcast_to_clients(const std::string& data) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    for (auto* conn : connections) {
        conn->send_text(data);
    }
}

void start_websocket_server() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/ws")
    .websocket(&app)
    .onopen([&](crow::websocket::connection& conn) {
        std::lock_guard<std::mutex> lock(connections_mutex);
        connections.push_back(&conn);
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        std::lock_guard<std::mutex> lock(connections_mutex);
        connections.erase(std::remove(connections.begin(), connections.end(), &conn), connections.end());
    });

    app.port(18080).multithreaded().run();
}

int main() {
    std::thread websocket_thread(start_websocket_server);

    // Initialisieren und Starten des OmniscopeSampler
    static constexpr int VID = 0x2e8au;
    static constexpr int PID = 0x000au;
    
    OmniscopeDeviceManager deviceManager{};
    std::vector<std::shared_ptr<OmniscopeDevice>> devices = deviceManager.getDevices(VID, PID);
    OmniscopeSampler sampler(deviceManager, std::move(devices));

    while (true) {
        std::map<Omniscope::Id, std::vector<std::pair<double, double>>> outMap;
        sampler.copyOut(outMap);

        // Serialisieren der Daten
        std::stringstream ss;
        for (const auto& [id, samples] : outMap) {
            ss << "Device ID: " << id.serial << "\n";
            for (const auto& sample : samples) {
                ss << "Time: " << sample.first << ", Value: " << sample.second << "\n";
            }
        }

        broadcast_to_clients(ss.str());

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    websocket_thread.join();

    return 0;
}

