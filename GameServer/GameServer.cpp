#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

vector <INT32> v;

// Mutual Exclusive (상호배타적)
mutex m;

// RAII (Resource Acquisition Is Initialization)
template<typename T>
class LockGuard
{
public:
	LockGuard(T& m)
	{
		_mutex = &m;
		_mutex->lock();
	}

	~LockGuard()
	{
		_mutex->unlock();
	}

private:
	T* _mutex;
};

void Push()
{
	// 자물쇠 잠그기 (재귀적 호출 불가)
	std::lock_guard<std::mutex> lockGuard(m);

	for (INT32 i = 0; i < 10000; i++)
	{
		//std::unique_lock<std::mutex> uniqueLock(m, std::defer_lock); // 잠그는 시점을 늦춰서 해줌

		//uniqueLock.lock();

		//m.lock();

		v.push_back(i);

		// 자물쇠 풀기
		//m.unlock();
	}
}

int main()
{
	std::thread t1(Push);
	std::thread t2(Push);

	t1.join();
	t2.join();

	cout << v.size() << endl;
}

