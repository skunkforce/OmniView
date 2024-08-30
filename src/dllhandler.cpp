#include "dllhandler.hpp"
#include <vector>
#include <nlohmann/json.hpp>

void* generalDllCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*)) {
    // DEBUG
    std::vector<int> callback_data{static_cast<int*>(data), static_cast<int*>(data) + size};
    fmt::print("Callback data: {}\n", nlohmann::json(callback_data).dump());

    deallocator(data);

    return nullptr;
}

DllHandler::DllHandler(const std::string& path) : dllPath(path) {}

bool DllHandler::load() {
    std::lock_guard<std::mutex> lock(dllMutex);
    dllHandle = dlopen(dllPath.c_str(), RTLD_LAZY);
    if (!dllHandle) {
        std::cerr << "Failed to load DLL: " << dlerror() << std::endl;
        return false;
    }

    initDllCallback = (InitDllCallbackFunc)dlsym(dllHandle, "init_callback");
    if (!initDllCallback) {
        std::cerr << "Failed to get init_callback function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    deinitDllCallback = (DeinitDllCallbackFunc)dlsym(dllHandle, "deinit_callback");
    if (!deinitDllCallback) {
        std::cerr << "Failed to get deinit_callback function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    // Init the DLL with a callback function
    initDllCallback(nullptr, nullptr);
    
    fmt::print("Dll loaded and initialized; {}\n", dllPath);
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
        fmt::print("DLL unloaded: {}\n", dllPath);
    }
}

DllHandler::~DllHandler() {
    unload();
}

