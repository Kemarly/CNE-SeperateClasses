#pragma once
#include <unordered_map>
#include <string>
#include <thread>
#include <set>
#include <chrono>
using namespace std;

class Server
{
private:
	unordered_map<string, string> userCredentials;
	const int MAX_CLIENTS = 5;
	WSADATA wsaData;
	set<SOCKET> connectedClients; 
	SOCKET udpSocket;
	sockaddr_in broadcastAddr;
	thread udpBroadcastThread;
	bool udpBroadcastRunning;
public:
	int init(int port, int capacity, char commandChar, int udpPort);
	int tcp_recv_whole(SOCKET s, char* buf, int len);
	int readMessage(char* buffer, int32_t size);
	int sendMessage(char* data, int32_t length);
	string GetHelpMessage();
	int RegisterUser(const string& username, const string& password);
	int HandleCommand(const string& command);
	int ProcessLogin(const string& username, const string& password, SOCKET clientSocket);
	int BroadcastMessage(const string& message, SOCKET senderSocket);
	void handleMultipleClients();
	int SendClientList(SOCKET clientSocket);
	int SavePublicMessage(const string& message);
	int initUDP(int udpPort);
	void startUDPBroadcast();
	void stopUDPBroadcast();
	void stop();
};