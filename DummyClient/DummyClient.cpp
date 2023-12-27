#include "pch.h"
#include <iostream>

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
	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr; // IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);

	// host to network short
	// Little-Endian vs Big-Endian
	// ex) 0x12345678 4바이트 정수
	// low [0x78][0x56][0x34][0x12] high < little
	// low [0x12][0x34][0x56][0x78] high < big = network

	if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Connect ErrorCode : " << errCode << endl;
		
		return 0;
	}

	// 연결 성공 이제부터 데이터 송수신 가능

	cout << "Connected To Server!" << endl;

	while (true)
	{
		// TODO
		char sendBuffer[100] = "Hello World!";

		int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
		if (resultCode == SOCKET_ERROR)
		{
			int32 errCode = ::WSAGetLastError();
			cout << "Send ErrorCode : " << errCode << endl;

			return 0;
		}

		cout << "Send Data! Len = " << sizeof(sendBuffer) << endl;

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

		this_thread::sleep_for(1s);
	}

	// 소켓 리소스 반환
	::closesocket(clientSocket);

	// winsock 종료
	::WSACleanup();
}
