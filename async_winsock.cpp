#include <Winsock2.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <conio.h>
#pragma comment(lib, "ws2_32.lib")

constexpr int BUFFER_SIZE = 1024;
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;
std::chrono::steady_clock::time_point t1;
bool b = true;
bool print = false;
fsec fs;

unsigned long long totalbytes = 0;

void CALLBACK CompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
        if(!b)
        {
        std::cout << "Sent total of " << totalbytes << " bytes to client taking " << fs.count() << " seconds\n";
        }
}

void handleClientConnections(SOCKET serverSocket, std::vector<SOCKET>& clientSockets, std::mutex& clientMutex) {
    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting client connection." << WSAGetLastError() << "\n";
            closesocket(serverSocket);
            WSACleanup();
            return;
        }
        if(b)  {
            b = false;
            t1 = Time::now();
        }
        std::lock_guard<std::mutex> lock(clientMutex);
        clientSockets.push_back(clientSocket);
    }
}

void sendDataToClients(const std::vector<SOCKET>& clientSockets, std::mutex& clientMutex) {
    char buffer[BUFFER_SIZE] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    WSABUF dataBuf;
    dataBuf.len = BUFFER_SIZE;
    dataBuf.buf = buffer;

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = WSACreateEvent();

    while (true) {
        std::lock_guard<std::mutex> lock(clientMutex);
        for (SOCKET clientSocket : clientSockets) {
            totalbytes += 1024;
            if (WSASend(clientSocket, &dataBuf, 1, nullptr, 0, &overlapped, CompletionRoutine) == SOCKET_ERROR) {
                int errorCode = WSAGetLastError();
                if (errorCode != WSA_IO_PENDING) {
                    std::cerr << "WSASend failed with error code: " << errorCode << "\n";
                    closesocket(clientSocket);
                    WSACleanup();
                    return;
                }
            }
        }
        auto t2 = Time::now();
        fs = t2 - t1;
        if(!b && print)
        { 
            std::cout << "Async : Sent total of " << totalbytes << " bytes to client taking " << fs.count() << " seconds\n";
            //std::cout << fs.count() << " seconds elapsed\n";
        }
        
        if (_kbhit()) {
            char key = _getch();
            if (key == 'v' || key == 'V') {
                print = !print;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating server socket" << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3000);
    serverAddr.sin_addr.s_addr= INADDR_ANY;
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding server socket." << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening on server socket." << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::mutex clientMutex;
    std::vector<SOCKET> clientSockets;
    std::thread clientThread(handleClientConnections, serverSocket, std::ref(clientSockets), std::ref(clientMutex));
    sendDataToClients(clientSockets, clientMutex);
    
    closesocket(serverSocket);
    for (SOCKET clientSocket : clientSockets) {
        closesocket(clientSocket);
    }

    WSACleanup();
    return 0;
}
