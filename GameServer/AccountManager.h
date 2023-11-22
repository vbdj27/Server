#pragma once

#include <mutex>

class AccountManager
{
	USE_LOCK;

public:
	void AccountThenPlayer();
	void Lock();
};

extern AccountManager GAccountManager;
