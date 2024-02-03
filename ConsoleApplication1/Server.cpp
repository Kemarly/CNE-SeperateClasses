#include "Server.h"
#include <iostream>
#include <thread>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cstring>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
using namespace std;
SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
SOCKET ComSocket = accept(listenSocket, NULL, NULL);

int Server::init(int port, int capacity, char commandChar, int udpPort)
{
	//Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "WSAStartup failed." << endl;
		return 0;
	}
	//udp
	if (!initUDP(udpPort)) { cout << "UDP initialization failed." << endl; return 0; }

	//socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) { cout << "Socket not created." << endl; WSACleanup();            
	printf("Success\n"); }

	//bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(listening, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Bind failed." << endl;
		closesocket(listening);
		WSACleanup();
		return 0;
	}

	//listen
	listen(listening, SOMAXCONN);
	cout << "Server listening: " << port << endl;

	//connection
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	char host[NI_MAXHOST];
	char service[NI_MAXHOST];
	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connect to port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connect to port " << ntohs(client.sin_port) << endl;
	}

	//close listen
	closesocket(listening);

	handleMultipleClients(); 

	//loop
	char buff[4096];
	while (true)
	{
		ZeroMemory(buff, 4096);
		//client send message
		int byteRecv = recv(clientSocket, buff, 4096, 0);
		if (byteRecv == SOCKET_ERROR) { cout << "Recive Error" << endl; }
		if (byteRecv == 0) { cout << "Disconnected" << endl; break; }
		send(clientSocket, buff, byteRecv * 1, 0);
	}

	//close sock
	closesocket(clientSocket);
	printf("Success\n");
}
int Server::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;
	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);
	return total;;
}

int Server::sendMessage(char* data, int32_t length)
{
	//Communication
	uint8_t recvSize = static_cast<uint8_t>(length);

	int result = tcp_recv_whole(ComSocket, (char*)&data, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnect\n");
	}
	char* buffer = new char[length];

	result = tcp_recv_whole(ComSocket, (char*)buffer, length);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnect\n");
	}
	delete[] buffer;
	printf("Success\n");
}
int Server::readMessage(char* buffer, int32_t size)
{
	uint8_t recvSize = 0;
	SOCKET ComSocket = accept(listenSocket, NULL, NULL);
	int result = tcp_recv_whole(ComSocket, (char*)&size, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnect\n");
	}
	buffer = new char[size];

	result = tcp_recv_whole(ComSocket, (char*)buffer, size);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnect\n");
	}
	delete[] buffer;
	printf("Success\n");
}


string Server::GetHelpMessage()
{
	string helpMessage = "Available commands:\n";
	helpMessage += "~help - Display available commands\n";
	helpMessage += "~register - username password - Register a new user\n";
	helpMessage += "~login - username password - Login with user account\n";
	helpMessage += "~send - Display available commands\n";
	helpMessage += "~getlist - Display available commands\n";
	helpMessage += "~logout - Logs out of user account\n";

	return helpMessage;
}
int Server::RegisterUser(const string& username, const string& password)
{
	if (userCredentials.size() >= MAX_CLIENTS)
	{
		string CAP_REACHED = "Max Client Capacity.";
		int CAPACITY_REACHED = stoi(CAP_REACHED);
		return CAPACITY_REACHED;
	}

	// Check if the username is already taken
	if (userCredentials.find(username) != userCredentials.end())
	{
		string USER_TAKEN = "Max Client Capacity.";
		int USERNAME_TAKEN = stoi(USER_TAKEN);
		return USERNAME_TAKEN;
	}

	// Register the user
	userCredentials[username] = password;
	printf("Success\n");
}
int Server::ProcessLogin(const string& username, const string& password, SOCKET clientSocket)
{
	return 0;
}
int Server::BroadcastMessage(const string& message, SOCKET senderSocket)
{
	return 0;
}
void Server::handleMultipleClients()
{
	fd_set master;
	FD_ZERO(&master);
	FD_SET(listenSocket, &master);

	while (true)
	{
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; ++i)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == listenSocket)
			{
				// Handle new connection
				sockaddr_in client;
				int clientSize = sizeof(client);
				SOCKET clientSocket = accept(listenSocket, (sockaddr*)&client, &clientSize);

				FD_SET(clientSocket, &master);
				connectedClients.insert(clientSocket);
			}
			else
			{
				// Handle data from existing client
				char buff[4096];
				int byteRecv = recv(sock, buff, 4096, 0);
				if (byteRecv == SOCKET_ERROR || byteRecv == 0)
				{
					// Client disconnected
					closesocket(sock);
					FD_CLR(sock, &master);
					connectedClients.erase(sock);
					// ... (existing code for displaying disconnection details)
				}
				else 
				{
					//get data
				}
			}
		}
	}

}
int Server::SendClientList(SOCKET clientSocket)
{
	return 0;
}
int Server::SavePublicMessage(const string& message)
{
	return 0;
}
int Server::initUDP(int udpPort) {
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET) {
		cout << "UDP Socket creation failed." << std::endl;
		return 0;
	}

	//make broadcast address 
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons(udpPort);
	broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	return 1;
}
void Server::startUDPBroadcast()
{
	udpBroadcastRunning = true;
	udpBroadcastThread = thread([this]()
		{
			while (udpBroadcastRunning) {
				//message
				string broadcastMessage = "Broadcasting from Server";

				//Send message
				sendto(udpSocket, broadcastMessage.c_str(), broadcastMessage.length(), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

				//wait x seconds
				this_thread::sleep_for(chrono::seconds(5));
			}
		});
	udpBroadcastThread.detach();
}

int Server::HandleCommand(const string& command)
{
	if (command.empty()) return 0;
	char commandChar = command[0];
	string args = command.substr(1);
	switch (commandChar)
	{
	case '~':

		if (args.find("help") != string::npos) {
			string helpMessage = GetHelpMessage();
			const char* helpChar = helpMessage.c_str();
			/*int helpLength = helpMessage.length();
			char* helpChar = new char[helpLength + 1];
			strncpy(helpChar, helpMessage.c_str(), helpLength);
			helpChar[helpLength] = '\0';
			for (int i = 0; i < helpLength; i++)
			{
				cout << helpChar[i];
			}
			sendMessage(helpChar, helpMessage.size());*/
			delete[] helpChar;
			printf("Success\n");
		}
		if (args.find("register") == 0)
		{
			// ~register
			size_t spacePos = args.find(' ');
			if (spacePos != string::npos)
			{
				string username = args.substr(9, spacePos - 9);
				string password = args.substr(spacePos + 1);
			}
		}
		else if (args.find("login") == 0)
		{
			//~login
			size_t spacePos = args.find(' ');
			if (spacePos != string::npos)
			{
				string username = args.substr(6, spacePos - 6);
				string password = args.substr(spacePos + 1);
				void ProcessLogin(const string & username, const string & password, SOCKET clientSocket);
			}
		}
		else if (args.find("send") == 0)
		{
			size_t spacePos = args.find(' ');

			void SendClientList(SOCKET clientSocket);
		}
		else if (args.find("getlist") == 0)
		{
			size_t spacePos = args.find(' ');

			// ~getlist
		}
		else if (args.find("logout") == 0)
		{
			size_t spacePos = args.find(' ');

			// ~logout
		}
		else
		{
			printf("Please enter a command");
		}
		break;
	default:
		BroadcastMessage(command, ComSocket); break;
	}
}
void Server::stopUDPBroadcast()
{
	udpBroadcastRunning = false;
	udpBroadcastThread.join();
	closesocket(udpSocket);
}
void Server::stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);
	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
	WSACleanup();
}