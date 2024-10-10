#include "dllhandler.hpp"
#include <vector>
#include <nlohmann/json.hpp>

/*
void* generalDllCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*)) {
    // DEBUG
    std::vector<int> callback_data{static_cast<int*>(data), static_cast<int*>(data) + size};
    fmt::print("Callback data: {}\n", nlohmann::json(callback_data).dump());

    deallocator(data);

    return nullptr;
}
*/

// Callback function that is used when the DLL sends back data
void* DllHandler::sawtoothCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*)) {
    std::vector<int> sawtooth_data{static_cast<int*>(data), static_cast<int*>(data) + size};

    // Output of the data received for verification
    fmt::print("In the memory address: {} , data: {}\n", static_cast<void*>(sawtooth_data.data()), nlohmann::json(sawtooth_data).dump());

    deallocator(data);  // Speicher freigeben
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

    initDllCallback = (InitDllCallbackFunc)dlsym(dllHandle, "init_sawtooth");
    if (!initDllCallback) {
        std::cerr << "Failed to get init_sawtooth function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    deinitDllCallback = (DeinitDllCallbackFunc)dlsym(dllHandle, "deinit_sawtooth");
    if (!deinitDllCallback) {
        std::cerr << "Failed to get deinit_sawtooth function: " << dlerror() << std::endl;
        dlclose(dllHandle);
        dllHandle = nullptr;
        return false;
    }

    // Init the DLL with a callback function
    initDllCallback(nullptr, nullptr);
    
    fmt::print("Dll loaded and initialized; {}\n", dllPath);
    return true;
}

void* DllHandler::callFunction(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*)) {
    std::lock_guard<std::mutex> lock(dllMutex);

    // Ensure that the DLL is loaded
    if (!dllHandle || !initDllCallback) {
        std::cerr << "DLL not loaded or initDllCallback not available!" << std::endl;
        return nullptr;
    }

    // Call the DLL function
    int result = initDllCallback(data, sawtoothCallback);

    if (result != 0) {
        std::cerr << "Error calling DLL function!" << std::endl;
        return nullptr;
    }

    // Return, if required
    return nullptr;
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
    std::cout << "Starting data transfer from DLL: " << dllPath << std::endl;

    // dllHandler.callFunction(...);

    dllHandler.unload();
    std::cout << "Data transfer from DLL completed." << std::endl;
}

