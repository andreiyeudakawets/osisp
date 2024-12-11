#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

void DispatchTask(HANDLE hPipe, const std::string& message) {
    DWORD bytesWritten;
    WriteFile(hPipe, message.c_str(), message.size(), &bytesWritten, NULL);
}

std::string ReceiveResult(HANDLE hPipe) {
    char buffer[512];
    DWORD bytesRead;
    ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    buffer[bytesRead] = '\0';
    return std::string(buffer);
}

// Функция для преобразования std::string в std::wstring
std::wstring StringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

int main() {
    //setlocale("russian");

    const int numWorkers = 3;
    std::vector<HANDLE> hPipes;
    std::vector<HANDLE> hProcesses;
    std::string pipeNameBase = "\\\\.\\pipe\\WorkerPipe";

    // Создание каналов и запуск рабочих процессов
    for (int i = 0; i < numWorkers; ++i) {
        std::string pipeName = pipeNameBase + std::to_string(i + 1);
        std::wstring widePipeName = StringToWString(pipeName); // Преобразуем в std::wstring
        HANDLE hPipe = CreateNamedPipe(
            widePipeName.c_str(), // Используем wide-строку
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 512, 512, 0, NULL
        );
        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Error creating pipe " << pipeName << std::endl;
            return 1;
        }
        hPipes.push_back(hPipe);

        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        std::wstring command = L"Worker.exe " + widePipeName;
        if (!CreateProcess(NULL, &command[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            std::cerr << "Error creating worker process " << i + 1 << std::endl;
            return 1;
        }
        hProcesses.push_back(pi.hProcess);

        ConnectNamedPipe(hPipe, NULL); 
    }

    std::vector<std::string> tasks = { "Hello World", "Distributed Computing", "Cryptography Test" };
    for (int i = 0; i < numWorkers; ++i) {
        DispatchTask(hPipes[i], tasks[i]);
    }

    for (int i = 0; i < numWorkers; ++i) {
        std::string result = ReceiveResult(hPipes[i]);
        std::cout << "Result from worker " << i + 1 << ": " << result << std::endl;
    }

    for (auto hPipe : hPipes) {
        CloseHandle(hPipe);
    }
    for (auto hProcess : hProcesses) {
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
    }

    return 0;
}
