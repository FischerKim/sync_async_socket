#include <Winsock2.h>
#include <iostream>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    bool print = true;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating client socket. Error code: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server. Error code: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    while (true) {
        char buffer[1024]; // Assuming a reasonable buffer size
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Error receiving data from server. Error code: " << WSAGetLastError() << "\n";
        } else {
            if (print) std::cout << "Received " << bytesRead << " bytes from server: " << buffer << "\n";
        }
        if (_kbhit()) {
            char key = _getch();
            if (key == 'v' || key == 'V') {
                print = !print;
            }
        }
    }

    // Clean up resources
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
