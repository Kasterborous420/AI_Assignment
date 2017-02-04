#include "Customer.h"

Customer::Customer()
{
}


Customer::~Customer()
{
}

// Customer state
void Customer::SetState(CUS_STATE state)
{
	this->customerState = state;
}

Customer::CUS_STATE Customer::GetState()
{
	return this->customerState;
}

// Customer position
void Customer::SetPos(MyVector pos)
{
	this->customerPos = pos;
}

MyVector Customer::GetPos()
{
	return this->customerPos;
}

// Customer spawn location (const)
MyVector Customer::GetSpawnLocation()
{
	return this->defaultSpawn;
}

// Customer line location (const)
MyVector Customer::GetLineLocation()
{
	return this->defaultSpawn;
}

// Customer seated
void Customer::SetSeated(bool seat)
{
	this->customerSeated = seat;
}

bool Customer::GetSeated()
{
	return this->customerSeated;
}

// Customer line
void Customer::SetLine(bool line)
{
	this->customerLine = line;
}

bool Customer::GetLine()
{
	return this->customerLine;
}

// Customer in line
void Customer::SetInLine(bool line)
{
	this->customerInLine = line;
}

bool Customer::GetInLine()
{
	return this->customerInLine;
}

// Customer back to spawn
void Customer::SetBackSpawn(bool spawn)
{
	this->backToSpawn = spawn;
}

bool Customer::GetBackSpawn()
{
	return this->backToSpawn;
}

// Customer speed
const float Customer::GetSpeed()
{
	return this->customerSpeed;
}

const float Customer::GetOrderTime()
{
	return this->orderTime;
}

const float Customer::GetEatTime()
{
	return this->eatTime;
}
