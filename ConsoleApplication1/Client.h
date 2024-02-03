#pragma once
#include <string>
#include <vector>
using namespace std;

class Client
{
private:
	SOCKET listenSocket;
	SOCKET ComSocket;
	const int MAX_CLIENTS = 5;
	vector<string> publicMessages;
public:
	int init(uint16_t port, char* address);
	int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
	int tcp_recv_whole(SOCKET s, char* buf, int len);
	int readMessage(char* buffer, int32_t size);
	int sendMessage(char* data, int32_t length);
	int HandleCommand(const string& command, SOCKET clientSocket);
	int ProcessLogin(const string& username, const string& password, SOCKET clientSocket);
	int BroadcastMessage(const string& message, SOCKET senderSocket);
	int SendClientList(SOCKET clientSocket);
	int SavePublicMessage(const string& message);
	void stop();
};