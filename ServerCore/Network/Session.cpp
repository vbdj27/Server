﻿#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session()
{
    _socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
    SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
    // 생각할 문제
    // 1) 버퍼 관리?
    // 2) sendEvent 관리? 단일? 여러개? WSAEvent 중첩?

    // TEMP
    SendEvent* sendEvent = xnew<SendEvent>();
    sendEvent->owner = shared_from_this(); // 레퍼런스 카운트 증가
    sendEvent->buffer.resize(len);
    ::memcpy(sendEvent->buffer.data(), buffer, len);

    WRITE_LOCK;
    RegisterSend(sendEvent);
}

void Session::Disconnect(const WCHAR* cause)
{
    if (_connected.exchange(false) == false)
        return;

    // TEMP
    wcout << "Disconnect : " << cause << endl;

    OnDisconnected(); //컨텐츠 코드에서 오버로딩
    SocketUtils::Close(_socket);
    GetService()->ReleaseSession(GetSessionRef());
}

HANDLE Session::GetHandle()
{
    return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
    switch(iocpEvent->eventType)
    {
    case EventType::Connect:
        ProcessConnect();
        break;
    case EventType::Recv:
        ProcessRecv(numOfBytes);
        break;
    case EventType::Send:
        ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
        break;
    default:
        break;
    }
}

void Session::RegisterConnect()
{
}

void Session::RegisterRecv()
{
    if (IsConnected() == false)
        return;

    _recvEvent.Init();
    _recvEvent.owner = shared_from_this(); // 레퍼런스 카운트 증가

    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
    wsaBuf.len = len32(_recvBuffer);

    DWORD numOfBytes = 0;
    DWORD flags = 0;
    if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr))
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            HandleError(errorCode);
            _recvEvent.owner = nullptr; // 레퍼런스 카운트 감소
        }
    }
}

void Session::RegisterSend(SendEvent* sendEvent)
{
    if (IsConnected() == false)
        return;

    WSABUF wsaBuf;
    wsaBuf.buf = (char*)sendEvent->buffer.data();
    wsaBuf.len = (ULONG)sendEvent->buffer.size();

    DWORD numOfBytes = 0;
    
   if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT &numOfBytes, 0, sendEvent, nullptr))
   {
       int32 errorCode = ::WSAGetLastError();
       if (errorCode != WSA_IO_PENDING)
       {
           HandleError(errorCode);
           sendEvent->owner = nullptr;
           xdelete(sendEvent);
       }
   }
}

void Session::ProcessConnect()
{
    _connected.store(true);

    // 세선 등록
    GetService()->AddSession(GetSessionRef());

    // 컨텐츠 로드에서 오버로딩
    OnConnected();

    // 수신 등록
    RegisterRecv();
}

void Session::ProcessRecv(int32 numOfBytes)
{
    _recvEvent.owner = nullptr; // 레퍼런스 카운트 감소

    if (numOfBytes == 0)
    {
        Disconnect(L"Recv 0");
        return;
    }

    // 컨텐츠 코드에서 오버로딩
    OnRecv(_recvBuffer, numOfBytes);

    // 수신 등록
    RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
    sendEvent->owner = nullptr;
    xdelete(sendEvent);

    if (numOfBytes == 0)
    {
        Disconnect(L"Send 0");
        return;
    }

    // 컨텐츠 코드에서 오버로딩
    OnSend(numOfBytes);
}

void Session::HandleError(int32 errorCode)
{
    switch (errorCode)
    {
    case WSAECONNRESET:
    case WSAECONNABORTED:
        Disconnect(L"HandleError");
        break;
    default:
        cout << "Handle Error : " << errorCode << endl;
        break;
    }
}


