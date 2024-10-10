#include "dllhandler.hpp"
#include <vector>
#include <nlohmann/json.hpp>

DllHandler::DllHandler(const std::string& path) : dllPath(path) {}

DllHandler::~DllHandler() {
    unload();
}

// Callback function that is used when the DLL sends back data
void* DllHandler::dllCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*)) {
    std::vector<int> callback_data{static_cast<int*>(data), static_cast<int*>(data) + size};

    // DEBUG
    fmt::print("In the memory address: {} , data: {}\n", static_cast<void*>(callback_data.data()), nlohmann::json(callback_data).dump());

    deallocator(data);
    return nullptr;
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

    // Init the DLL with a callback function
    initDllCallback(nullptr, dllCallback);
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

void DllHandler::startDllDataTransfer(const std::string& dllPath) {
    DllHandler dllHandler(dllPath);

    if (!dllHandler.load()) {
        std::cerr << "Failed to load DLL: " << dllPath << std::endl;
        return;
    }
    // DEBUG
    std::cout << "Starting data transfer to WebSocket" << std::endl;

    dllHandler.unload();
    // DEBUG
    std::cout << "Data transfer from DLL completed." << std::endl;
}

