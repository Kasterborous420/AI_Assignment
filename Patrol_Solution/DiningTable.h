#pragma once              
#include "MyVector.h"
class CDiningTable
{
private:
	MyVector pos;
	bool isEating;
	unsigned int tableSize;
public:

	void SetPos(MyVector _pos);
	MyVector GetPos();

	void GetStatus();
	void SetStatus();

	void SetSize();
	int GetSize();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
	CDiningTable();
	~CDiningTable();
};

