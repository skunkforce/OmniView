#include "dllhandler.hpp"
#include <vector>
#include <nlohmann/json.hpp>

// Callback function that is used when the DLL sends back data
void* dllCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*)) {
    std::vector<int> callback_data{static_cast<int*>(data), static_cast<int*>(data) + size};

    // DEBUG
    fmt::print("In the memory address: {} , data: {}\n", static_cast<void*>(callback_data.data()), nlohmann::json(callback_data).dump());

    // Send the DLL data to the WebSocket
    DllContext* context = static_cast<DllContext*>(data);
    if (context && context->wsHandler) {
        context->wsHandler->sendDllData(callback_data, timestamp);
    }

    // Release memory - check whether the deallocator is secure
    if (deallocator) {
        deallocator(data);
    } else {
        fmt::print("WARNING: No deallocator found, memory has not been released!\n");
    }
    return nullptr;
}

DllHandler::DllHandler(const std::string& path, WebSocketHandler* wsHandlerParam) : dllPath(path), wsHandler(wsHandlerParam),  context{wsHandlerParam} {}

DllHandler::~DllHandler() {
    unload();
}

bool DllHandler::load() {

    std::lock_guard<std::mutex> lock(dllMutex);
    dllHandle = dlopen(dllPath.c_str(), RTLD_LAZY);
    if (!dllHandle) {
        std::cerr << "Failed to load DLL: " << dlerror() << std::endl;
        return false;
    }

    initDllCallback = (InitDllCallbackFunc)dlsym(dllHandle, "init_sawtooth");
    if (!initDllCallback) {
        std::cerr << "Failed to get init_sawtooth function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    // Create DllContext and assign WebSocketHandler
    // DllContext context {wsHandler};

    // Init the DLL with a callback function
    initDllCallback(static_cast<void*>(&context), dllCallback);
    // DEBUG
    fmt::print("Dll loaded and initialized; {}\n", dllPath);

    deinitDllCallback = (DeinitDllCallbackFunc)dlsym(dllHandle, "deinit_sawtooth");
    if (!deinitDllCallback) {
        std::cerr << "Failed to get deinit_sawtooth function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    return true;
}

void DllHandler::unload() {
    std::lock_guard<std::mutex> lock(dllMutex);
    if (dllHandle) {
        if (deinitDllCallback) {
            deinitDllCallback(dllHandle);
        }
        dlclose(dllHandle);
        dllHandle = nullptr;
        // DEBUG
        fmt::print("DLL unloaded: {}\n", dllPath);
    }
}

void DllHandler::searchDlls(const std::string& searchPath) {
    std::string dllExtension = ".so";

    std::cout << "Searching for DLLs in: " << searchPath << std::endl;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(searchPath)) {
        if (entry.path().extension() == dllExtension) {
            std::cout << " Found DLL: " << entry.path().filename().string() << std::endl;
        }
    }
}

void DllHandler::startDllDataTransfer(const std::string& dllPath, WebSocketHandler* wsHandler) {
    DllHandler dllHandler(dllPath, wsHandler);

    // Assign the WebSocket handler to the instance
    // dllHandler.wsHandler = wsHandler;

    if (!dllHandler.load()) {
        std::cerr << "Failed to load DLL: " << dllPath << std::endl;
        return;
    }
    // DEBUG
    std::cout << "Starting data transfer to WebSocket" << std::endl;

    // Start des WebSocket-Threads für die DLL-Daten
    wsHandler->startWebSocketThreadForDll(dllHandler.context.dllData);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    dllHandler.unload();
    // DEBUG
    std::cout << "Data transfer from DLL completed." << std::endl;
}

