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

// 참고) CV는 User Level Object (커널 오브젝트 X)
condition_variable cv;

void Producer()
{
	while (true)
	{
		// 1) Lock을 잡고
		// 2) 공유 변수 값을 수정
		// 3) Lock을 풀고
		// 4) 조건변수 통해 다른 스레드에게 통제

		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}
		

		cv.notify_one(); // wait중인 스레드가 있으면 1개만 깨운다.

		this_thread::sleep_for(100ms);

	}

	
}

void Consumer()
{
	while (true)
	{

		unique_lock<mutex> lock(m)
		cv.wait(lock, []() {return q.empty() == false; });

		// 1) Lock을 잡고
		// 2) 조건 확인
		// 3) 만족 => 빠져나와서 다음코드 진행
		//    불만족 => Lock을 풀어주고 대기 상태로 전환

		// 그런데 notify_one을 했으면 항상 조건식을 만족하는거 아닐까?
		// Spurious WakeUp (가짜 기상?)
		// notify_one을 할 때 lock을 잡고 있는 것이 아니기 때문

		//while (q.empty() == false)
		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
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

