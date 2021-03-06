/// Emil Hedemalm
/// 2014-02-14
/// A message to set the values of a certain vector.

#include "MathMessage.h"

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

VectorMessage::VectorMessage(String message, Vector2f vectorValue)
	: Message(MessageType::VECTOR_MESSAGE), vectorType(VECTOR_2F), vec2f(vectorValue)
{
	this->msg = message;
}

VectorMessage::VectorMessage(String message, const Vector3f & vectorValue)
: Message(MessageType::VECTOR_MESSAGE), vectorType(VECTOR_3F), vec3f(vectorValue)
{
	this->msg = message;
}

VectorMessage::VectorMessage(String message, const Vector4f & vectorValue)
: Message(MessageType::VECTOR_MESSAGE), vectorType(VECTOR_4F), vec4f(vectorValue)
{
	this->msg = message;
}

VectorMessage::~VectorMessage()
{

}	

// Returns the values embedded within in the Vector4f format, converting as necessary.
Vector4f VectorMessage::GetVector4f()
{
	Vector4f vec;
	switch(vectorType)
	{
		case VECTOR_2I:
			vec = Vector4f(vec2i[0], vec2i[1],0,1);
			break;
		case VECTOR_3F:
			return vec3f;
		case VECTOR_4F:
			return vec4f;
		default:
			assert(false);
	}
	return vec;
}

// Returns the values embedded within in the Vector4f format, converting as necessary.
Vector3f VectorMessage::GetVector3f()
{
	Vector3f vec;
	switch(vectorType)
	{
		case VECTOR_2I:
			return vec2i;
		case VECTOR_2F:
			return vec2f;
		case VECTOR_3F:
			return vec3f;
		case VECTOR_4F:
			return vec4f;
		default:
			assert(false);
	}
	return vec;
}

Vector2f VectorMessage::GetVector2f()
{
	Vector2f vec(0,0);
	switch(vectorType)
	{
		case VECTOR_2I:
			return vec2i;
		case VECTOR_2F:
			return vec2f;
		case VECTOR_3F:
			return vec3f;
		case VECTOR_4F:
			return Vector2f(vec4f.x, vec4f.y);
		default:
			assert(false);
	}
	return vec;	
}

	