#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

#pragma comment(lib, "winmm.lib")  // Подключаем библиотеку для использования timeGetTime

using namespace std;

// Количество операций в каждом потоке
const int OPERATION_COUNT = 1500;

// Структура для передачи параметров потоку
struct ThreadData {
    int threadNum;
    HANDLE logFileHandle;
    DWORD startTime; // Время начала работы программы
};

// Функция потока
DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    int num = data->threadNum;
    HANDLE logFileHandle = data->logFileHandle;
    DWORD startTime = data->startTime;

    // Цикл, выполняющий заданное количество операций
    for (int i = 0; i < OPERATION_COUNT; i++) {
        DWORD currentTime = timeGetTime();  // Получаем текущее время
        DWORD elapsedTime = currentTime - startTime; // Вычисляем время выполнения операции

        // Формируем строку с результатами
        std::ostringstream oss;
        oss << elapsedTime << " \n";
        std::string output = oss.str();

        // Записываем строку в файл
        DWORD bytesWritten;
        WriteFile(logFileHandle, output.c_str(), output.size(), &bytesWritten, NULL);
    }

    ExitThread(0);
}

int main(int argc, char* argv[])
{
    const int threadCount = 2;

    // Открываем файл для записи данных
    HANDLE logFileHandle = CreateFile(L"thread_log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (logFileHandle == INVALID_HANDLE_VALUE) {
        cerr << "Error opening file for logging" << endl;
        return 1;
    }

    HANDLE handles[threadCount];
    ThreadData threadData[threadCount];

    // Получаем начальное время
    DWORD startTime = timeGetTime();

    // Создаем потоки
    for (int i = 0; i < threadCount; i++) {
        threadData[i] = { i + 1, logFileHandle, startTime };  // Инициализируем данные потока
        handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadData[i], 0, NULL);
        if (handles[i] == NULL) {
            cerr << "Error creating thread" << endl;
            CloseHandle(logFileHandle);
            return 1;
        }
    }

    // Устанавливаем более высокий приоритет для первого потока
    //SetThreadPriority(handles[0], THREAD_PRIORITY_HIGHEST);

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(threadCount, handles, TRUE, INFINITE);

    // Закрываем дескрипторы потоков
    for (int i = 0; i < threadCount; i++) {
        CloseHandle(handles[i]);
    }

    // Закрываем файл
    CloseHandle(logFileHandle);

    cout << "Logging completed. Check 'thread_log.txt' for results." << endl;
    return 0;
}
