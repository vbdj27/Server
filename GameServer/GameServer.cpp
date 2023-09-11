#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

// Momory Model (정책)
// 1) Sequentially Consistant (seq_cst)
// 2) Acquire-Release (acquire, release)
// 3) Relaxed(relaxed)

// 1) seq_cst (가장 엄격 = 컴파일러 최적화 여지 적음 = 직관적)
// -> 가시성 문제해결, 코드 재배치 해결

// 2) acquire-release
// -> 중간! relese 명령 이전의 메모리 명령들이, 해당 명령 이후로 재배치 되는 것을 금지
// -> acquire로 같은 변수를 읽는 스레드가 있다면 release 이전의 명령들이 acquire 순간에 관찰 가능 (가시성 보장)

// 3) relaxed (자유로움 = 컴파일러 최적화 여지 많음 = 직관적이지 않음)
// -> 코드 재배치 컴파일러 맘대로, 가시성 해결 x (동일 객체에 대한 동일 관찰 순서만 보장)



atomic<bool> flag;
atomic<bool> ready;
int32 value;

void Producer()
{
	value = 10;

	ready.store(true, memory_order_seq_cst);
}

void Consumer()
{
	while (ready.load(memory_order_seq_cst) == false)

	;
		
	cout << value << endl;
}

int main()
{
	flag = false;

	flag.store(true, memory_order_seq_cst);

	bool val = flag.load(memory_order_seq_cst);

	// 이전 flag 값을 prev에 넣고, flag 값을 수정
	{
		bool prev = flag.exchange(true);
	}

	// CAS(Compare And Swap) 조건부 수정
	{
		bool expected = false;
		bool desired = true;
		flag.compare_exchange_strong(expected, desired);

		if (flag == expected)
		{
			flag = desired;
			return true;
		}

		else
		{
			expected = flag;
			return false;
		}
	}

	ready = false;
	value = 0;
	thread t1(Producer);
	thread t2(Consumer);
	t1.join();
	t2.join();
}

