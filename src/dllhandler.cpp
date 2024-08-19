#include "dllHandler.hpp"

DllHandler::DllHandler(const std::string& path) : dllPath(path) {}

bool DllHandler::load() {
    std::lock_guard<std::mutex> lock(dllMutex);
    dllHandle = dlopen(dllPath.c_str(), RTLD_LAZY);
    if (!dllHandle) {
        std::cerr << "Failed to load DLL: " << dlerror() << std::endl;
        return false;
    }

    initSawtooth = (InitSawtoothFunc)dlsym(dllHandle, "init_sawtooth");
    if (!initSawtooth) {
        std::cerr << "Failed to get init_sawtooth function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    deinitSawtooth = (DeinitSawtiithFunc)dlsym(dllHandle, "deinit_sawtooth");
    if (!deinitSawtooth) {
        std::cerr << "Failed to get deinit_sawtooth function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    // Init the DLL with a callback function
    initSawtooth(nullptr, nullptr);
    
    fmt::print("Dll loaded and initialized; {}\n", dllPath);
    return true;
}

void DllHandler::unload() {
    std::lock_guard<std::mutex> lock(dllMutex);
    if (dllHandle) {
        if (deinitSawtooth) {
            deinitSawtooth(dllHandle);
        }
        dlclose(dllHandle);
        dllHandle = nullptr;
        fmt::print("DLL unloaded: {}\n", dllPath);
    }
}

DllHandler::~DllHandler() {
    unload();
}

void* DllHandler::callFunction(void* data, size_t size, void* dev_handle, size_t timestamp, void (*deallocator)(void*)) {
    if (initSawtooth) {
        return initSawtooth(data, size, dev_handle, timestamp, deallocator);
    }
    return nullptr;
}
