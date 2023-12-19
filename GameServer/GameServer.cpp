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
#include "Allocator.h"



int main()
{
	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
				
				}
			}
		);
	}

	GThreadManager->Join();
}

