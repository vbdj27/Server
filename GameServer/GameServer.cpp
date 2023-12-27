#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <winSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main()
{
	// winsock 초기화 (ws2_32 라이브러리 초기화)
// 관련 정보가 wsaData에 채워짐
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// ad : Address Family(AF_INET =IPv4, AF_INET^ = IPv6)
	// type : TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
	// protocol : 0
	// return : decriptor
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr; // IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // 알아서 설정
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

	if (::listen(listenSocket, 10) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Listen ErrorCode : " << errCode << endl;
		return 0;
	}

	while (true)
	{
		SOCKADDR_IN clientAddr; // IPv4
		::memset(&serverAddr, 0, sizeof(serverAddr));
		int32 addrLen = sizeof(clientAddr);

		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			int32 errCode = ::WSAGetLastError();
			cout << "Accept ErrorCode : " << errCode << endl;
			return 0;
		}

		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client Connected IP = " << ipAddress << endl;

		while (true)
		{
			char recvBuffer[1000];

			int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen <= 0)
			{
				int32 errCode = ::WSAGetLastError();
				cout << "Recv ErrorCode : " << errCode << endl;

				return 0;
			}

			cout << "Recv Data Data = " << recvBuffer << endl;
			cout << "Recv Data Len = " << recvLen << endl;

			int32 resultCode = ::recv(clientSocket, recvBuffer, sizeof(recvLen), 0);
			if (resultCode == SOCKET_ERROR)
			{
				int32 errCode = ::WSAGetLastError();
				cout << "Send ErrorCode : " << errCode << endl;

				return 0;
			}
		}
	}

	// winsock 종료
	::WSACleanup();
}

