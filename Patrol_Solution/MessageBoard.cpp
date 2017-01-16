#include "MessageBoard.h"

MessageBoard::MessageBoard()
{
	Reset();
}


MessageBoard::~MessageBoard()
{
}

void MessageBoard::setMessage(string msg)
{
	message = msg;
}

void MessageBoard::setLabel_From(string from)
{
	from_label = from;
}

void MessageBoard::setLabel_To(string to)
{
	to_label = to;
}

string MessageBoard::getMessage()
{
	return message;
}

string MessageBoard::getLabel_From()
{
	return from_label;
}

string MessageBoard::getLabel_To()
{
	return to_label;
}

void MessageBoard::Reset()
{
	message = from_label = to_label = "Nil";
}
