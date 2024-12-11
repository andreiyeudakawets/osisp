#include <winsock2.h>
#include <ws2tcpip.h> // Для inet_pton
#include <iostream>
#include <string>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

struct Message {
    char sender[32];
    char recipient[32];
    char body[256];
};

void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }
}

void clientFunction(const std::string& serverIp) {
    initializeWinsock();

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000); // Порт сервера
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "Connected to server!" << std::endl;

    // Поток для приема сообщений
    std::thread receiveThread([clientSocket]() {
        char buffer[512];
        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                std::cout << "Message: " << std::string(buffer, 0, bytesReceived) << std::endl;
            }
        }
        });

    // Отправка сообщений
    /*
    
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        send(clientSocket, input.c_str(), input.size(), 0);
    }
    */
    std::string username;
    std::cout << "Enter your name: ";
    std::cin >> username;
    std::cin.ignore();

    while (true) {
        Message msg = {};
        strncpy_s(msg.sender, username.c_str(), sizeof(msg.sender) - 1);

        std::cout << "Recipient (or 'all'): ";
        std::string recipient;
        std::getline(std::cin, recipient);
        strncpy_s(msg.recipient, recipient.c_str(), sizeof(msg.recipient) - 1);

        std::cout << "Message: ";
        std::string text;
        std::getline(std::cin, text);
        strncpy_s(msg.body, text.c_str(), sizeof(msg.body) - 1);

        // Отправка сообщения
        send(clientSocket, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
    }


    receiveThread.join();
    closesocket(clientSocket);
    WSACleanup();
}

int main() {
    std::string serverIp;
    std::cout << "Enter server IP (e.g., 127.0.0.1): ";
    std::cin >> serverIp;
    std::cin.ignore(); // Игнорируем символ новой строки после ввода
    clientFunction(serverIp);
    return 0;
}
