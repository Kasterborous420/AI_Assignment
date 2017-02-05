#pragma once
#include "MyVector.h"
#include <vector>
using std::vector;

class CWaiter
{
public: 

	// Waiter related
	enum WAITER_STATE
	{
		E_WAITER_SERVE,
		E_WAITER_IDLE,
		E_WAITER_PICKUP,
		E_WAITER_PICKUPCUSTOMER,
		E_WAITER_MOVE,
		E_WAITER_MAX
	};

private:

	WAITER_STATE waiterState;
	MyVector waiterPos;
	MyVector waiterSpawn;
	bool foodReady;
	bool availableCustomers;
	bool customerPickup;
	bool isBusy;
	float waiterSpeed = 0.15f;


public:
	CWaiter();
	~CWaiter();

	vector <MyVector> waiterWayPoints;

	WAITER_STATE GetState();
	void SetState(WAITER_STATE newState);

	MyVector GetPos();
	void SetPos(MyVector newPos);

	MyVector GetSpawn();
	void SetSpawn(MyVector newSpawn);

	bool GetFoodReady();
	void SetFoodReady(bool newFoodReady);

	bool GetAvailableCustomers();
	void SetAvailableCustomers(bool newAvailableCustomers);

	bool GetCustomerPickup();
	void SetCustomerPickup(bool newCustomerPickup);
	
	bool GetBusy();
	void SetBusy(bool busyornot);

	const float GetSpeed();



};

