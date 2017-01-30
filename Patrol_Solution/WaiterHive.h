#pragma once
#include <vector>
#include "Waiter.h"

using std::vector;

class CWaiterHive
{

private:
	static CWaiterHive *instance;
	vector<CWaiter>Roster;
	vector<CWaiter>::iterator iter;

public:
	CWaiterHive();
	~CWaiterHive();

#pragma region Variables and Shit
	// Vector of waiters

#pragma endregion


#pragma region Functions
	//Get Instance of this class
	static CWaiterHive *GetInstance();

	CWaiter GetWaiter();
	void AddWaiter(CWaiter waiter);

#pragma endregion
};

