#include "Waiter.h"


CWaiter::CWaiter()
{
}


CWaiter::~CWaiter()
{
}

CWaiter::WAITER_STATE CWaiter::GetState()
{
	return this->waiterState;
}

void CWaiter::SetState(CWaiter::WAITER_STATE newState)
{
	this->waiterState = newState;
}

MyVector CWaiter::GetPos()
{
	return this->waiterPos;
}

void CWaiter::SetPos(MyVector newPos)
{
	this->waiterPos = newPos;
}

MyVector CWaiter::GetSpawn()
{
	return this->waiterSpawn;
}

void CWaiter::SetSpawn(MyVector newSpawn)
{
	this->waiterSpawn = newSpawn;
}

bool CWaiter::GetFoodReady()
{
	return this->foodReady;
}

void CWaiter::SetFoodReady(bool newFoodReady)
{
	this->foodReady = newFoodReady;
}

bool CWaiter::GetAvailableCustomers()
{
	return this->availableCustomers;
}

void CWaiter::SetAvailableCustomers(bool newAvailableCustomers)
{
	this->availableCustomers = newAvailableCustomers;
}

bool CWaiter::GetCustomerPickup()
{
	return this->customerPickup;
}
void CWaiter::SetCustomerPickup(bool newCustomerPickup)
{
	this->customerPickup = newCustomerPickup;
}

bool CWaiter::GetBusy()
{
	return this->isBusy;
}

void CWaiter::SetBusy(bool busy)
{
	this->isBusy = busy;
}

const float CWaiter::GetSpeed()
{
	return this->waiterSpeed;
}