#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    Server();
    ~Server();

    void Start();
    void UDPBroadcast();
    static const int UDP_BROADCAST_PORT = 12345; // Choose a port number
    static const int UDP_BROADCAST_INTERVAL = 10; // Interval in seconds

    std::string serverIP;
    int serverPort;


private:
    std::mutex clientMutex;
    SOCKET listenSocket;
    fd_set masterset, readyset;
    std::unordered_map<std::string, std::string> userCredentials;

    static const char DEFAULT_COMMAND_CHAR = '~';
    static const int MAX_BUFFER_SIZE = 256;
    static const int MAX_CLIENTS = 5;

    std::string GetHelpMessage();
    int RegisterUser(const std::string& username, const std::string& password);
    int tcp_recv_whole(SOCKET s, char* buf, int len);
    int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
    void BroadcastMessage(const std::string& message);
    void ProcessLogin(const std::string& username, const std::string& password, SOCKET clientSocket);
    std::string GetClientList();
    void HandleCommand(const std::string& command);
    void HandleClient(SOCKET clientSocket, fd_set& readSet);
    void ServerCode();
        void UDPBroadcast();
};

