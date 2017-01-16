#pragma once
#include <string>
#include <iostream>

using namespace std;

class MessageBoard
{
public:
	MessageBoard();
	~MessageBoard();

	string message, from_label, to_label;

	void setMessage(string msg);
	void setLabel_From(string from);
	void setLabel_To(string to);

	string getMessage();
	string getLabel_From();
	string getLabel_To();

	void Reset();
};

