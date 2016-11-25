#pragma once              
#include "MyVector.h"
class CDiningTable
{
private:
	MyVector pos;
	bool isOccupied;
	unsigned int tableSize;
public:

	void SetPos(MyVector _pos);
	MyVector GetPos();

	bool GetStatus();
	void SetStatus(bool isEat);

	void SetSize(unsigned int size);
	int GetSize();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
	CDiningTable();
	~CDiningTable();
};

