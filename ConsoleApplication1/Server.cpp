#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "Server.h"
using namespace std;
int tcp_recv_whole(SOCKET s, char* buf, int len)
{
    int total = 0;
    int ret = recv(s, buf, 1, 0);
    if (ret <= 0)
        return ret;
    uint8_t size = static_cast<uint8_t>(*buf);
    do
    {
        int ret = recv(s, buf + total, len - total, 0);
        if (ret < 1)
            return ret;
        else
            total += ret;

    } while (total < len);
    return total;
}
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
    int result;
    result = send(skSocket, reinterpret_cast<const char*>(&length), 1, 0);
    if (result <= 0)
        return result;
    int bytesSent = 0;
    while (bytesSent < length)
    {
        result = send(skSocket, data + bytesSent, length - bytesSent, 0);
        if (result <= 0)
            return result;
        bytesSent += result;
    }
    return bytesSent;
}
Server::Server() {
    // Initialize Winsock
    WSADATA wsadata;
    if (WSAStartup(WINSOCK_VERSION, &wsadata) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {
    // Cleanup Winsock
    closesocket(listenSocket);
    WSACleanup();
}

void Server::Start() {
    //Create listening socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        cerr << "Failed to create socket." << endl;
        return;
    }

    //server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    cout << "Enter TCP Port number: ";
    int port;
    cin >> port;
    serverAddr.sin_port = htons(port);

    cout << "Enter chat capacity (maximum number of clients): ";
    int maxClients;
    cin >> maxClients;

    cout << "Enter command character (default is ~): ";
    char commandChar;
    cin >> commandChar;

    //bind
    int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        cerr << "Failed to bind socket." << endl;
        closesocket(listenSocket);
        return;
    }

    //listening for incoming connections
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cerr << "Failed to listen on socket." << endl;
        closesocket(listenSocket);
        return;
    }

    //Obtain server host IP using gethostname() and getaddrinfo()
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    cout << "Server running on host: " << hostname << endl;
    struct addrinfo* info;
    getaddrinfo(hostname, nullptr, nullptr, &info);

    char ip[INET6_ADDRSTRLEN];
    for (auto addr = info; addr != nullptr; addr = addr->ai_next) {
        if (addr->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr->ai_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip, sizeof(ip));
            cout << "IPv4 Address: " << ip << endl;
        }
        else if (addr->ai_family == AF_INET6) {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr->ai_addr;
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ip, sizeof(ip));
            cout << "IPv6 Address: " << ip << endl;
        }
    }
    freeaddrinfo(info);
    cout << "Listening on port: " << port << endl;

    //multiplexing
    FD_ZERO(&masterset);
    FD_SET(listenSocket, &masterset);
    cout << "Waiting for connections..." << endl;

    while (true)
    {
        // Copy the master set to the ready set
        readyset = masterset;

        // Use select to wait for activity on the sockets
        int temp = select(0, &readyset, NULL, NULL, NULL);
        if (temp == SOCKET_ERROR) { cerr << "Select function failed: " << WSAGetLastError() << endl;  break; }

        // Check for new connections on the listening socket
        if (FD_ISSET(listenSocket, &readyset))
        {
            // Accept the new connection
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) { cerr << "Failed to accept connection." << endl; }
            else if (masterset.fd_count < FD_SETSIZE)
            {
                cout << "New connection accepted." << endl;

                // Send the welcome message
                const char* welcomeMessage = "Welcome to the Server!\n Please enter your commands starting with (~): ";
                tcp_send_whole(clientSocket, welcomeMessage, strlen(welcomeMessage));

                FD_SET(clientSocket, &masterset);
            }
            else
            {
                cout << "Maximum number of clients reached. Connection rejected." << endl;
                closesocket(clientSocket);
            }
        }

        // Check for activity on the connected sockets
        for (u_int i = 0; i < masterset.fd_count; ++i)
        {
            SOCKET currentSocket = masterset.fd_array[i];
            if (currentSocket != listenSocket && FD_ISSET(currentSocket, &readyset))
            {
                //Handle client message
                HandleClient(currentSocket, masterset);
            }
        }
    }
    //Close listening socket
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
}

void Server::UDPBroadcast() {
    // Create a UDP socket
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        cerr << "Failed to create UDP socket." << endl;
        return;
    }

    // Enable broadcast option
    int broadcastOption = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastOption, sizeof(broadcastOption)) == SOCKET_ERROR)
    {
        cerr << "Failed to enable broadcast option." << endl;
        closesocket(udpSocket);
        return;
    }

    // Construct the broadcast address structure
    sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(UDP_BROADCAST_PORT);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    // Compose the broadcast message
    string broadcastMessage = "Server IP: " + serverIP + ", Port: " + to_string(serverPort);

    while (true)
    {
        // Send the broadcast message
        if (sendto(udpSocket, broadcastMessage.c_str(), broadcastMessage.length(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR)
        {
            cerr << "Failed to send broadcast message." << endl;
        }

        // Sleep for x seconds before sending the next broadcast
        this_thread::sleep_for(chrono::seconds(UDP_BROADCAST_INTERVAL));
    }
}