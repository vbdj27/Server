#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

mutex m;
queue<int32> q;
HANDLE handle;

void Producer()
{
	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}
		

		::SetEvent(handle); //시그널 변경

		this_thread::sleep_for(100ms);

	}

	
}

void Consumer()
{
	while (true)
	{
		::WaitForSingleObject(handle, INFINITE); // handle의 시그널 상태에 따라 동작변경
		// ::ResetEvent(handle);

		unique_lock<mutex> lock(m);

		if (q.empty() == false)
		{
			int32 data = q.front();
			q.pop();
			cout << data << endl;
		}
	}
}

int main()
{
	// 커널 오브젝트
	// Usage Count
	// Signal(파란불) / Non-Signal(빨간불)
	// Auto / Manual

	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*ManualReset 자동 수동*/, FALSE, NULL);

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}

