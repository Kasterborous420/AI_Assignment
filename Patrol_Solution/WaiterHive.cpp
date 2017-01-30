#include "WaiterHive.h"

CWaiterHive *CWaiterHive::instance;

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
	for (iter = Roster.begin(); iter != Roster.end(); iter++)
	{
		if (!(iter->GetBusy))
		{
			return *iter;
		}
	}
}

void CWaiterHive::AddWaiter(CWaiter waiter)
{
	this->Roster.push_back(waiter);
}

