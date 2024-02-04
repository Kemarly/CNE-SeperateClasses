#pragma once
#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <chrono>

class Client {
public:
    Client();
    ~Client();

    void Connect(const std::string& serverIP, int serverPort);
    void Start();
    void ReceiveMessages();
    void SendMessage(const std::string& message);

private:
    SOCKET clientSocket;
    std::thread receiveThread;

    static const int MAX_BUFFER_SIZE = 256;

    int tcp_recv_whole(SOCKET s, char* buf, int len);
    int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
    void HandleReceivedMessage(const char* buffer);
};

