#include "Chef.h"

Chef::Chef()
{
}


Chef::~Chef()
{
}

void Chef::SetState(CHEF_STATE state)
{
	this->chefState = state;
}

Chef::CHEF_STATE Chef::GetState()
{
	return this->chefState;
}

const float Chef::GetSpeed()
{
	return this->chefSpeed;
}

const float Chef::GetCookTime()
{
	return this->cookTime;
}

void Chef::SetPos(MyVector pos)
{
	this->chefPos = pos;
}

MyVector Chef::GetPos()
{
	return this->chefPos;
}

void Chef::SetSpawn(MyVector spawn)
{
	this->chefSpawn = spawn;
}

MyVector Chef::GetSpawn()
{
	return this->chefSpawn;
}

void Chef::SetStation(MyVector station)
{
	this->chefStation = station;
}

MyVector Chef::GetStation()
{
	return this->chefStation;
}

void Chef::SetAtStation(bool station)
{
	this->isAtStation = station;
}

bool Chef::GetAtStation()
{
	return this->isAtStation;
}

void Chef::SetArrived(bool arrived)
{
	this->chefArrived = arrived;
}

bool Chef::GetArrived()
{
	return this->chefArrived;
}
