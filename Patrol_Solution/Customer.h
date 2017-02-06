#pragma once

#include "MyVector.h"
#include <vector>
#include <time.h>
#include "Waiter.h"
using std::vector;

class Customer
{
public:

	// Customer state
	enum CUS_STATE
	{
		E_CUSTOMER_IDLE,
		E_CUSTOMER_QUEUE,
		E_CUSTOMER_MOVE,
		E_CUSTOMER_ORDER,
		E_CUSTOMER_EAT,
		E_CUSTOMER_LEAVE,
		E_CUSTOMER_MAX
	};

private:

	CUS_STATE customerState;
	const float customerSpeed = 0.15f;
	const float orderTime = 3.f;
	const float eatTime = 10.f;
	bool customerSeated;
	bool customerLine;
	bool customerInLine;
	bool backToSpawn;
	MyVector customerPos;
	const MyVector defaultSpawn = MyVector(12.f, 0.f);
	const MyVector defaultLine = MyVector(8.5f, 3.5f);
	CWaiter* assignedWaiter;

public:

	clock_t orderStart, orderEnd, eatStart, eatEnd;

	
	Customer();
	~Customer();

	void SetState(CUS_STATE state);
	CUS_STATE GetState();

	void SetPos(MyVector pos);
	MyVector GetPos();

	void SetSeated(bool seat);
	bool GetSeated();

	void SetLine(bool line);
	bool GetLine();

	void SetInLine(bool line);
	bool GetInLine();

	void SetBackSpawn(bool spawn);
	bool GetBackSpawn();

	const float GetSpeed();
	const float GetOrderTime();
	const float GetEatTime();

	// Get const MyVectors
	MyVector GetSpawnLocation();
	MyVector GetLineLocation();

	CWaiter* GetAssignedWaiter();
	void SetAssignedWaiter(CWaiter* waiter);
};

