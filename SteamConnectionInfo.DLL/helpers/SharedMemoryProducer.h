#pragma once
#include <Windows.h>
#include <string>

class SharedMemoryProducer {

private:
    HANDLE file;
    LPVOID address;
    HANDLE producerReadyEvent;
    HANDLE consumerReadyEvent;
    int size;
    std::string error;
public:
    SharedMemoryProducer() {
        file = INVALID_HANDLE_VALUE;
        address = nullptr;
        producerReadyEvent = INVALID_HANDLE_VALUE;
        consumerReadyEvent = INVALID_HANDLE_VALUE;
        size = 4096;
        error = "";
    }
    std::string GetLastError() const {
        return error;
    }
    void SetLastError(const std::string& errorToSet) {
        error = errorToSet;
    }
    bool Initialize(const std::string& sharedMemoryRegionName, const int& sharedMemoryRegionSize) {
        if (sharedMemoryRegionName.empty() || sharedMemoryRegionName.size() > MAX_PATH) {
            SetLastError("Initialize() failed: The name of the shared memory region must not be empty and have fewer than 260 characters.");
            return false;
        }

        if (sharedMemoryRegionSize < 64 || sharedMemoryRegionSize > 4096) {
            SetLastError("Initialize() failed: The size of the shared memory region must be between 64 and 4096 bytes.");
            return false;
        }

        size = sharedMemoryRegionSize;

        file = CreateFileMapping(
            INVALID_HANDLE_VALUE,   // Use the page file as the backing store
            NULL,                   // Default security attributes
            PAGE_READWRITE,         // Read/write access
            0,                      // Size of the file mapping object
            sharedMemoryRegionSize,                   // Size of the shared memory region (in bytes)
            sharedMemoryRegionName.c_str());      // Name of the shared memory region

        if (file == NULL)
        {
            SetLastError("CreateFileMapping() failed: " + GetLastError());
            return false;
        }

        // Map the shared memory region into the process
        address = MapViewOfFile(
            file,               // Handle to the file mapping object
            FILE_MAP_ALL_ACCESS,    // Read/write access
            0,                      // Offset to the start of the mapped view
            0,                      // Number of bytes to map (0 means map the entire file)
            0);                     // Map the entire file

        if (address == NULL)
        {
            SetLastError("MapViewOfFile() failed: " + GetLastError());
            CloseHandle(file);
            return false;
        }

        producerReadyEvent = CreateEvent(
            NULL,   // Default security attributes
            TRUE,   // Manual reset event
            FALSE,  // Initially not signaled
            "ProducerReady"); // Name of the event

        if (producerReadyEvent == NULL)
        {
            SetLastError("CreateEvent() for ProducerReady event failed: " + GetLastError());
            UnmapViewOfFile(address);
            CloseHandle(file);
            return false;
        }

        consumerReadyEvent = CreateEvent(
            NULL,   // Default security attributes
            TRUE,   // Manual reset event
            FALSE,  // Initially not signaled
            "ConsumerReady"); // Name of the event

        if (consumerReadyEvent == NULL)
        {
            SetLastError("CreateEvent() for ConsumerReady event failed: " + GetLastError());
            UnmapViewOfFile(address);
            CloseHandle(file);
            return false;
        }

        return true;
    }

    bool ConsumerIsReady() {
        ResetEvent(producerReadyEvent);

        DWORD dwWaitResult = WaitForSingleObject(consumerReadyEvent, 1000);
        if (dwWaitResult == WAIT_OBJECT_0) {
            ResetEvent(consumerReadyEvent);
            return true;
        }
        return false;
    }

    void Wait(const int& ms) {
        Sleep(ms);
    }
    void SetData(const std::string& data) {
        ZeroMemory(address, size);
        CopyMemory(address, data.c_str(), data.size());

        SetEvent(producerReadyEvent);
    }
    void Destroy() {
        if (address)
            UnmapViewOfFile(address);

        if (file)
            CloseHandle(file);

        if (producerReadyEvent)
            CloseHandle(producerReadyEvent);

        if (consumerReadyEvent)
            CloseHandle(consumerReadyEvent);
    }

    ~SharedMemoryProducer() {
        Destroy();
    }
};