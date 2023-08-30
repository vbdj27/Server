#pragma once

#include <mutex>

class Account
{
	// TODO
};

class AccountManager
{
public:
	static AccountManager* Instance()
	{
		static AccountManager instance;
		return &instance;
	}

	Account* GetAccount(INT32 id)
	{
		lock_guard<mutex>  guard(_mutex);

		// ¹º°¡¸¦ °®°í ¿È
		return nullptr;
	}

	void ProcessLogin();

private:
	mutex _mutex;
	map<INT32, Account*> _accounts;
};

