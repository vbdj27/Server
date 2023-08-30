#include "pch.h"
#include "UserManager.h"
#include "AccountManager.h"

void UserManager::ProcessSave()
{
	//account Lock
	Account* account = AccountManager::Instance()->GetAccount(100);

	// userlock
	lock_guard<mutex> guard(_mutex);

	
}