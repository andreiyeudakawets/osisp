// Worker.cpp
#include <windows.h>
#include <iostream>
#include <string>

// Функция для преобразования std::string в std::wstring
std::wstring StringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}


std::string SimpleXOREncryptDecrypt(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        //c ^= 0xAA; // XOR encryption
        c ^= 'A';
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Worker must receive the pipe name as an argument" << std::endl;
        return 1;
    }

    // Получение имени канала из аргументов командной строки
    std::string pipeName = argv[1];

    std::wstring widePipeName = StringToWString(pipeName);

    HANDLE hPipe = CreateFile(
        widePipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to pipe." << std::endl;
        return 1;
    }

    char buffer[512];
    DWORD bytesRead;
    ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    buffer[bytesRead] = '\0';

    std::string response = SimpleXOREncryptDecrypt(buffer);

    DWORD bytesWritten;
    WriteFile(hPipe, response.c_str(), response.size(), &bytesWritten, NULL);

    CloseHandle(hPipe);
    return 0;
}
