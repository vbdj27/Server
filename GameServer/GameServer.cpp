﻿#include "pch.h"
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

void HandleError(const char* cause)
{
	int32 errCode = ::WSAGetLastError();
	cout << cause << "ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000;

struct Session
{
	SOCKET socket;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
	int32 sendBytes = 0;
};

int main()
{
	// winsock 초기화 (ws2_32 라이브러리 초기화)
	// 관련 정보가 wsaData에 채워짐
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// 블로킹(Blocking) 소켓
	// accept -> 접속한 클라가 있을 때
	// connect -> 서버 접속 성공했을 때
	// send, sendto -> 요청한 데이터를 송신 버퍼에 복사했을 떄
	// recv, recvfrom -> 수신 버퍼에 도착한 데이터가 있고, 이를 유저러벨 버퍼에 복사했을 떄

	// 논블로킹(Non-Blocking)
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << endl;

	// WSAEventSelect = (WSAEventSelect 함수가 핵심이되는)
	// 소켓과 관련된 네트워크 이벤트를 [이벤트 객체]를 통해 감지

	// 이벤트 객체 관련 함수들
	// 생성 : WSACreateEvent (수동 리셋 Manual-Reset + Non-signaled 상태 시작)
	// 삭제 : WSACloseEvent
	// 신호 상태 감지 : WSAWaitForMultipleEvents
	// 구체적인 네트워크 이벤트 알아내기 : WSAEnumNetworkEvents

	// 소켓<-> 이벤트 객체 연동
	// WSAEventSelect(socket, event, networkEvents)
	// - 관심있는 네트워크 이벤트
	// FD_ACCEPT : 접속한 클라가 있음 accept
	// FD_READ : 데이터 수신 가능 recv, recvfrom
	// FD_CLOSE : 데이터 송신 가능 send, sendto
	// FD_CONNECT : 통신을 위한 연결 절차 완료
	// FD_OOB

	// 주의 사항
	// WSAEventSelect 함수를 호출하면, 해당 소켓은 자동으로 넌블로킹 모드 전환
	// accept() 함수가 리턴하는 소켓은 listenSocket과 동일한 속성을 갖는다
	// -따라서 clientSocket은 FD_READ, FD_WRITE 등을 다시 등록 필요
	// 드물게 WSAEWOULDBLOCK 오류가 뜰 수 있으니 예외 처리 필요
	// 중요!
	// - 이벤트 발생 시, 적절한 소켓 함수 호출해야 함
	// - 아니면 다음 번에는 동일 네트워크 이벤트가 발생 X
	// ex) FD_READ 이벤트 떴으면 recv() 호출해야 하고, 안하면 FD_READ 두 번 다시 X

	// 1) count, event
	// 2) waitAll : 모두 기다림? 하나만 완료 되어도 OK?
	// 3) timeout : 타임아웃
	// 4) 지금은 false
	// return : 완료된 첫번쨰 인덱스
	// WSAWaitForMultpleEvents

	// 1) socket
	// 2) eventObject : socket과 연동된 이벤트 객체 핸들을 넘겨주면, 이벤트 객체를 non-signaled
	// 3) networkEvent : 네트워크 이벤트 / 오류 정보가 저장
	// WSAEnumnetwork Events

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;
	sessions.reserve(100);

	WSAEVENT listenEvent = ::WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{ listenSocket });
	if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
		return 0;

	fd_set reads;
	fd_set writes;

	while (true)
	{
		int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
			continue;

		index -= WSA_WAIT_EVENT_0;

		//::WSAResetEvent(wsaEvents[index]);

		WSANETWORKEVENTS networkEvents;
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
			continue;

		// Listener 소켓 체크
		if (networkEvents.lNetworkEvents & FD_ACCEPT)
		{
			// Error-Check
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				continue;

			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);

			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client Connected" << endl;

				WSAEVENT clientEvent = ::WSACreateEvent();
				wsaEvents.push_back(clientEvent);
				sessions.push_back(Session{ clientSocket });
				if (::WSAEventSelect(listenSocket, listenEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
					return 0;
			}
		}

		// Client Session 소켓 체크
		if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE)
		{
			// Error-Check
			if ((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT] != 0))
				continue;
			// Error-Check
			if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
				continue;

			Session& s = sessions[index];

			// Read
			if (s.recvBytes == 0)
			{
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					// TODO : Remove Session
					continue;
				}

				s.recvBytes = recvLen;
				cout << "Recv Data = " << recvLen << endl;
			}

			// Write
			if (s.recvBytes > s.sendBytes)
			{
				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
				if (sendLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					// TODO : Remove Session
					continue;
				}

				s.sendBytes += sendLen;
				if (s.recvBytes == s.sendBytes)
				{
					s.recvBytes = 0;
					s.sendBytes = 0;
				}

				cout << "Send Data = " << sendLen << endl;
					
			}
		}

		// FD_CLOSE 처리
		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			// TODO : Remove Session
		}
	}

	// winsock 종료
	::WSACleanup();
}

