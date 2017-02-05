#pragma once

#include "MyVector.h"
#include <vector>
#include <time.h>
using std::vector;

class Caller
{
public:

	enum CALLER_STATE
	{
		E_CALLER_IDLE,
		E_CALLER_MOVE,
		E_CALLER_CLEAR,
		E_CALLER_SPAWN,
		E_CALLER_MAX
	};

private:

	CALLER_STATE callerState;
	MyVector callerPos;
	const float waiterSpeed = 0.15f;
	const float clearTime = 5.f;
	const MyVector callerSpawn = MyVector(6.25f, 3.5f);
	bool clearTable;
	bool arriveTable;
	bool backToSpawn;

public:
	Caller();
	~Caller();

	clock_t clearStart, clearEnd;

	vector <MyVector> callerWaypoints;

	void SetState(CALLER_STATE state);
	CALLER_STATE GetState();

	MyVector GetSpawnLocation();

	void SetPos(MyVector pos);
	MyVector GetPos();

	void SetClear(bool clear);
	bool GetClear();

	const float GetSpeed();
	const float GetClearTime();

	void SetArrive(bool arrive);
	bool GetArrive();

	void SetBackSpawn(bool spawn);
	bool GetBackSpawn();
};

