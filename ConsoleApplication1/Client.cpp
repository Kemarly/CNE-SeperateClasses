#include "Client.h"
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
Client::Client() {
    // Initialize Winsock
    WSADATA wsadata;
    if (WSAStartup(WINSOCK_VERSION, &wsadata) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create a TCP socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

Client::~Client() {
    // Cleanup Winsock
    closesocket(clientSocket);
    WSACleanup();
}

void Client::Connect(const std::string& serverIP, int serverPort) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

void Client::Start() {
    const char* welcomeMessage = "Welcome to the Server!\n Please enter your commands starting with (~): ";
    tcp_send_whole(clientSocket, welcomeMessage, strlen(welcomeMessage));
    while (true)
    {
        uint8_t size = 0;
        int result = tcp_recv_whole(clientSocket, reinterpret_cast<char*>(&size), 1);
        if ((result == SOCKET_ERROR) || (result == 0))
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf("  recv disconnected client\n");
            FD_CLR(clientSocket, &readSet);
            closesocket(clientSocket);
            break;
        }

        char* buffer = new char[size];
        result = tcp_recv_whole(clientSocket, buffer, size);
        if ((result == SOCKET_ERROR) || (result == 0))
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf("  Client stopped connection\n");
            FD_CLR(clientSocket, &readSet);
            closesocket(clientSocket);
            delete[] buffer;
            break;
        }
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf(" Received a message from a client\n");
            printf("\n\n");
            printf("%s", buffer);
            printf("\n\n");

            // Set the command variable with the received message
            string command = string(buffer);
            HandleCommand(command);
        }
        delete[] buffer;
    }
}
void Client::SendMessage(const std::string& message) {
    int bytesSent = send(clientSocket, message.c_str(), message.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Failed to send message." << std::endl;
    }
}
void Client::ReceiveMessages() {
    // Implement the message receiving logic here
    // ...
}

void Client::SendMessage(const std::string& message) {
    // Implement the message sending logic here
    // ...
}

