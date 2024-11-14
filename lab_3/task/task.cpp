#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>

constexpr int NUM_OPERATIONS = 100;

struct ThreadData {
    int threadId;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::ofstream* logFile;
    HANDLE* semaphore;
};

void SomeLongOperation() {
    for (int i = 0; i < 10000000; ++i) {
        const auto k = i * i;
        i += k;
        for (int o = 0; o < k; ++o) {
            i -= k;
        }
    }
}

DWORD WINAPI ThreadFunction(const LPVOID param) {
    const auto* data = static_cast<ThreadData*>(param);
    const int threadId = data->threadId;
    std::ofstream& logFile = *(data->logFile);
    const HANDLE* semaphore = data->semaphore;

    const auto startTime = data->startTime;

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        SomeLongOperation();
        auto currentTime = std::chrono::high_resolution_clock::now();
        const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        WaitForSingleObject(*semaphore, INFINITE);
        logFile << std::format("{}\t{}\n", threadId, elapsedTime);
        ReleaseSemaphore(*semaphore, 1, nullptr);
    }

    return 0;
}

int main() {
    std::string temp;
    std::cin >> temp;
    std::ofstream logFile("log.txt");

    if (!logFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return 1;
    }

    HANDLE semaphore = CreateSemaphore(nullptr, 1, 1, nullptr);
    if (semaphore == nullptr) {
        std::cerr << "Error: Unable to create semaphore" << std::endl;
        return 1;
    }

    auto* handles = new HANDLE[2];
    const auto startTime = std::chrono::high_resolution_clock::now();
    ThreadData data1 = { 1, startTime, &logFile, &semaphore };
    ThreadData data2 = { 2, startTime, &logFile, &semaphore };

    handles[0] = CreateThread(nullptr, 0, ThreadFunction, &data1, 0, nullptr);
    handles[1] = CreateThread(nullptr, 0, ThreadFunction, &data2, 0, nullptr);

    SetThreadPriority(handles[0], THREAD_PRIORITY_TIME_CRITICAL);
    SetThreadPriority(handles[1], THREAD_PRIORITY_NORMAL);

    WaitForMultipleObjects(2, handles, TRUE, INFINITE);

    delete[] handles;
    CloseHandle(semaphore);

    return 0;
}