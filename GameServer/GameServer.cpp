#include "pch.h"
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"
#include "RefCounting.h"
#include "Memory.h"
#include "Allocator.h"
#include <iostream>


class Knight
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	};

	~Knight()
	{
		cout << "~Knight()" << endl;
	};

	int32 _hp = 100;
	int32 _mp = 10;
};

// new operator overloading (Global)
void* operator new(size_t size)
{
	cout << "new! " << size << endl;
	void* ptr = ::malloc(size);
	return ptr;
}

void operator delete(void* ptr)
{
	cout << "delete !"  << endl;
	::free(ptr);
}

void* operator new[](size_t size)
{
	cout << "new[] " << size << endl;
	void* ptr = ::malloc(size);
	return ptr;
}
void operator delete[](void* ptr)
{
	cout << "delete! []" << endl;
	::free(ptr);
}

int main()
{
	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Vector<Knight> v(10);

					Map<int32, Knight> m;
					m[100] = Knight();

					this_thread::sleep_for(10ms);
				}
			});
	}

	GThreadManager->Join();
}

