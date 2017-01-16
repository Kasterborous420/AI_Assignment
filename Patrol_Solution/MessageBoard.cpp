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
	msg = message;
}

void MessageBoard::setLabel_From(string from)
{
	from = from_label;
}

void MessageBoard::setLabel_To(string to)
{
	to = to_label;
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
}
