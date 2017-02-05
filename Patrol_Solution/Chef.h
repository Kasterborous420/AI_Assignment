#pragma once

#include "MyVector.h"
#include <vector>
using std::vector;

class Chef
{
public:

	enum CHEF_STATE
	{
		E_CHEF_COOK,
		E_CHEF_SERVE,
		E_CHEF_WAIT,
		E_CHEF_MAX
	};

private:

	CHEF_STATE chefState;
	const float chefSpeed = 0.02f;
	const float cookTime = 5.0f;
	MyVector chefPos;
	MyVector chefSpawn;
	MyVector chefStation;
	bool isAtStation;
	bool chefArrived;

public:

	Chef();
	~Chef();

	vector<MyVector> chefWaypoints;

	void SetState(CHEF_STATE state);
	CHEF_STATE GetState();
	
	const float GetSpeed();
	const float GetCookTime();

	void SetPos(MyVector pos);
	MyVector GetPos();

	void SetSpawn(MyVector spawn);
	MyVector GetSpawn();

	void SetStation(MyVector station);
	MyVector GetStation();

	void SetAtStation(bool station);
	bool GetAtStation();

	void SetArrived(bool arrived);
	bool GetArrived();
};

