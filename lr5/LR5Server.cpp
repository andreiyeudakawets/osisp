#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")


struct Message {
    char sender[32];      // Имя отправителя или идентификатор (максимум 32 символа)
    char recipient[32];   // Имя получателя (или "all" для рассылки всем)
    char body[256];       // Основное сообщение (максимум 256 символов)
};

void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }
}

void serverFunction() {
    initializeWinsock();

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // позволяет принимать подключения на любом доступном IP - адресе.

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
    // SOMAXCONN задаёт максимальное количество ожидаемых соединений.
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Server is running on port 54000..." << std::endl;

    std::vector<SOCKET> clients;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket != INVALID_SOCKET) {
            clients.push_back(clientSocket);
            std::cout << "New client connected!" << std::endl;

            // Обработка клиента в отдельном потоке
            /*
            std::thread([clientSocket, &clients]() {
                char buffer[512];
                while (true) {
                    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesReceived <= 0) break;

                    std::string message(buffer, bytesReceived);
                    std::cout << "Received: " << message << std::endl;

                    // Рассылка сообщения всем клиентам
                    for (SOCKET s : clients) {
                        if (s != clientSocket) {
                            send(s, buffer, bytesReceived, 0);
                        }
                    }
                }
                closesocket(clientSocket);
                }).detach();
            */
            std::thread([clientSocket, &clients]() {
                char buffer[sizeof(Message)];
                while (true) {
                    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesReceived <= 0) break;

                    // Десериализация сообщения
                    Message* msg = reinterpret_cast<Message*>(buffer);

                    std::cout << "Message from " << msg->sender << " to "
                        << msg->recipient << ": " << msg->body << std::endl;

                    // Рассылка сообщения всем клиентам или адресному
                    for (SOCKET s : clients) {
                        if (s != clientSocket) {
                            send(s, buffer, sizeof(Message), 0);
                        }
                    }
                }
                closesocket(clientSocket);
                }).detach();
        }
    }
    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    serverFunction();
    return 0;
}
