#include "WaiterHive.h"


CWaiterHive::CWaiterHive()
{
	CWaiterHive::instance = NULL;
}


CWaiterHive::~CWaiterHive()
{
}

CWaiterHive* CWaiterHive::GetInstance()
{
	if (!instance)
	{
		instance = new CWaiterHive();
	}
	return instance;
}

CWaiter CWaiterHive::GetWaiter()
{
	if (FreeWaiters.size() > 0)
	{
		// Create waiter
		CWaiter waiter;
		// Assign first available waiter 
		waiter = FreeWaiters[0];
		// Remove that fucker from the list cuz he not free
		FreeWaiters.erase(FreeWaiters.begin());
		return waiter;
	}
}

void CWaiterHive::AddWaiter(CWaiter waiter)
{
	this->Roster.push_back(waiter);
	if (!(waiter.GetBusy()))
	{
		FreeWaiters.push_back(waiter);
	}
}

void CWaiterHive::FreeUp(CWaiter waiter)
{
	FreeWaiters.push_back(waiter);
}
