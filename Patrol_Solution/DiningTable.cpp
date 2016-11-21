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
	if (isOccupied)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CDiningTable::SetStatus(bool occupy)
{
	this->isOccupied = occupy;
}

void CDiningTable::SetSize(unsigned int size)
{
	this->tableSize = size;
}

int CDiningTable::GetSize()
{
	return tableSize;
}
