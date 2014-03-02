/// Emil Hedemalm
/// 2014-02-14
/// A message to set the values of a certain vector.

#include "VectorMessage.h"
#include "MathLib.h"

FloatMessage::FloatMessage(String message, float value)
:Message(MessageType::FLOAT_MESSAGE), value(value)
{
	this->msg = message;
}
FloatMessage::~FloatMessage()
{
}

VectorMessage::VectorMessage(String message)
: Message(MessageType::VECTOR_MESSAGE)
{
	this->msg = message;
}

VectorMessage::~VectorMessage()
{

}	