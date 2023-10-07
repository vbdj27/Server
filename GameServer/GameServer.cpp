#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

CoreGlobal Core;

void ThreadMain()
{
	while (true)
	{
		cout << "Hello ! I am thread...";
	}
}

int main()
{
	
}

