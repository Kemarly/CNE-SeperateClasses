#include "Client.h"
#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
using namespace std;

int Client::init(uint16_t port, char* address)
{
	//Socket
	if (ComSocket == INVALID_SOCKET)
	{
		ComSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ComSocket == INVALID_SOCKET) { printf("Connection Error\n"); }
	
	}

	//Connect
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	if (inet_pton(AF_INET, address, &serverAddr.sin_addr) <= 0) { printf("address Error\n"); }


	int result = connect(ComSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR) {
		printf("Connection Error\n"); }
	printf("Success\n");
}

int Client::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);
		if (result <= 0)
			return result;
		bytesSent += result;
	}
	return bytesSent;
}
int Client::tcp_recv_whole(SOCKET s, char* buf, int len)
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

int Client::readMessage(char* buffer, int32_t size)
{
	//Communication
	char sendbuffer[30];
	memset(sendbuffer, 0, 30);

	int result = tcp_recv_whole(ComSocket, sendbuffer, size);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnect\n");
	}

	result = tcp_recv_whole(ComSocket, buffer, size);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		//delete[] sendbuffer;
		printf("Paramiter error\n");
	}
	//delete[] sendbuffer;
	printf("Success\n");
}
int Client::sendMessage(char* data, int32_t length)
{
	//Communication
	uint8_t messageSize = static_cast<uint8_t>(length);
	int result = send(ComSocket, (char*)&messageSize, sizeof(messageSize), 0);

	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("shutdown\n");
	}

	result = send(ComSocket, data, length, 0);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnect\n");
	}
	printf("Success\n");
}

int Client::ProcessLogin(const string& username, const string& password, SOCKET clientSocket)
{

}
int Client::BroadcastMessage(const string& message, SOCKET senderSocket)
{

}
int Client::SendClientList(SOCKET clientSocket)
{

}
int Client::SavePublicMessage(const string& message)
{

}

int Client::HandleCommand(const string& command, SOCKET clientSocket)
{
	if (command.empty())	printf("Success\n");


	char commandChar = command[0];
	string args = command.substr(1);
	switch (commandChar)
	{
	case '~':
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
				ProcessLogin(username, password, clientSocket);
			}
		}
		else if (args.find("send") == 0)
		{
			SendClientList(clientSocket);
		}
		else if (args.find("getlist") == 0)
		{
			// ~getlist 
		}
		else if (args.find("logout") == 0)
		{
			// ~logout
		}
		else
		{

		}
		break;
	default:
		BroadcastMessage(command, clientSocket); break;
	}
}

void Client::stop()
{
	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
	WSACleanup();
}