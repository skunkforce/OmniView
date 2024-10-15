#ifndef DLLHANDLER_HPP
#define DLLHANDLER_HPP

#include "websockethandler.hpp"

#include <string>
#include <dlfcn.h>
#include <iostream>
#include <fmt/core.h>
#include <mutex>

struct DllContext {
    WebSocketHandler* wsHandler;
    std::vector<int> dllData;
};

void* dllCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*));

// Function pointer for the DLL-Function

typedef int (*InitDllCallbackFunc)(void*, void* (*)(void*, size_t, void*, size_t, void (*)(void*)));
typedef int (*DeinitDllCallbackFunc)(void*);

class DllHandler {
public:
    DllHandler(const std::string& dllPath, WebSocketHandler* wsHandlerParam);
    ~DllHandler();

    bool load();
    void unload();

    static void searchDlls(const std::string& searchPath);
    static void startDllDataTransfer(const std::string& dllPath, WebSocketHandler* wsHandler);

    // static void* dllCallback(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*));

private:
    std::string dllPath;
    void* dllHandle = nullptr;
    InitDllCallbackFunc initDllCallback = nullptr;
    DeinitDllCallbackFunc deinitDllCallback = nullptr;
    std::mutex dllMutex;
    WebSocketHandler* wsHandler;
    DllContext context;
};

#endif

