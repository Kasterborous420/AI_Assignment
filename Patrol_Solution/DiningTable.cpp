#include "DiningTable.h"


CDiningTable::CDiningTable()
{
}


CDiningTable::~CDiningTable()
{
}

void CDiningTable::SetPos(MyVector _pos)
{
	this->pos = _pos;
}

MyVector CDiningTable::GetPos()
{
	return pos;
}

bool CDiningTable::GetStatus()
{
	if (isEating)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CDiningTable::SetStatus(bool isEat)
{
	this->isEating = isEat;
}

void CDiningTable::SetSize(unsigned int size)
{
	this->tableSize = size;
}

int CDiningTable::GetSize()
{
	return tableSize;
}
