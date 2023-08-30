#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>

// atomic  : All or Nothing

atomic<INT32> sum = 0; //병목현상 발생할 수 있음 (연산느림)

void Add()
{
	for (INT32 i = 0; i < 1000000; i++)
	{
		sum.fetch_add(1);
	}
}

void Sub()
{
	for (INT32 i = 0; i < 1000000; i++)
	{
		sum.fetch_add(-1);
	}
}

int main()
{
	Add();
	Sub();

	cout << sum << endl;
	
	std::thread t1(Add);
	std::thread t2(Sub);
	t1.join();
	t2.join();
	
	cout << sum << endl;
	
}

