#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"
#include "RefCounting.h"
#include "Memory.h"

class Knight
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}

	~Knight()
	{
		cout << "~Knight()" << endl;
	}
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
	Knight* knight = xnew<Knight>(100);

	xdelete(knight);
}

