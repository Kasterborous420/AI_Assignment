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

MyVector Caller::GetSpawnLocation()
{
	return this->callerSpawn;
}
