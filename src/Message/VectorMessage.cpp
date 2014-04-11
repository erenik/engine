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

IntegerMessage::IntegerMessage(String message, int value)
	: Message (MessageType::INTEGER_MESSAGE), value(value)
{
	this->msg = message;
}
IntegerMessage::~IntegerMessage()
{
}


VectorMessage::VectorMessage(String message, Vector2i vectorValue)
: Message(MessageType::VECTOR_MESSAGE), vectorType(VECTOR_2I), vec2i(vectorValue)
{
	this->msg = message;
}

VectorMessage::VectorMessage(String message, Vector3f vectorValue)
: Message(MessageType::VECTOR_MESSAGE), vectorType(VECTOR_3F), vec3f(vectorValue)
{
	this->msg = message;
}

VectorMessage::VectorMessage(String message, Vector4f vectorValue)
: Message(MessageType::VECTOR_MESSAGE), vectorType(VECTOR_4F), vec4f(vectorValue)
{
	this->msg = message;
}

VectorMessage::~VectorMessage()
{

}	