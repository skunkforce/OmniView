#ifndef DLLHANDLER_HPP
#define DLLHANDLER_HPP

#include <string>
#include <dlfcn.h>
#include <iostream>
#include <fmt/core.h>
#include <mutex>

// Function pointer for the DLL-Function

typedef int (*InitSawtoothFunc)(void*, void* (*)(void*, size_t, void*, size_t, void (*)(void*)));
typedef int (*DeinitSawtoothFunc)(void*);

class DllHandler {
public:
    DllHandler(const std::string& dllPath);
    ~DllHandler();

    bool load();
    void unload();
    void* callFunction(void* data, size_t size, void*, size_t timestamp, void (*deallocator)(void*));

private:
    std::string dllPath;
    void* dllHandle = nullptr;
    InitSawtoothFunc initSawtooth = nullptr;
    DeinitSawtoothFunc deinitSawtooth = nullptr;
    std::mutex dllMutex;
};

#endif

