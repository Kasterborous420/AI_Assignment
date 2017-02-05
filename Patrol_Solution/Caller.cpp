#include "Caller.h"

Caller::Caller()
{
}


Caller::~Caller()
{
}

void Caller::SetState(CALLER_STATE state)
{
	this->callerState = state;
}

Caller::CALLER_STATE Caller::GetState()
{
	return this->callerState;
}

void Caller::SetPos(MyVector pos)
{
	this->callerPos = pos;
}

MyVector Caller::GetPos()
{
	return this->callerPos;
}

void Caller::SetClear(bool clear)
{
	this->clearTable = clear;
}

bool Caller::GetClear()
{
	return this->clearTable;
}

const float Caller::GetSpeed()
{
	return this->waiterSpeed;
}

const float Caller::GetClearTime()
{
	return this->clearTime;
}

void Caller::SetArrive(bool arrive)
{
	this->arriveTable = arrive;
}

bool Caller::GetArrive()
{
	return this->arriveTable;
}

void Caller::SetBackSpawn(bool spawn)
{
	this->backToSpawn = spawn;
}

bool Caller::GetBackSpawn()
{
	return this->backToSpawn;
}

MyVector Caller::GetSpawnLocation()
{
	return this->callerSpawn;
}
