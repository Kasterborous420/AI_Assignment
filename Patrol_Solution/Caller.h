#pragma once

#include "MyVector.h"
#include <vector>
using std::vector;

class Caller
{
public:

	enum CALLER_STATE
	{
		E_CALLER_IDLE,
		E_CALLER_CHECK,
		E_CALLER_WAIT,
		E_CALLER_CALL,
		E_CALLER_MAX
	};

private:

	CALLER_STATE callerState;
	MyVector callerPos;
	const MyVector callerSpawn = MyVector(6.25f, 3.5f);

public:
	Caller();
	~Caller();

	void SetState(CALLER_STATE state);
	CALLER_STATE GetState();

	void SetPos(MyVector pos);
	MyVector GetPos();

	MyVector GetSpawnLocation();
};

