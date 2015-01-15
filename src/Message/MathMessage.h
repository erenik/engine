/// Emil Hedemalm
/// 2014-02-14
/// A message to set the values of a certain vector.

#ifndef VECTOR_MESSAGE_H
#define VECTOR_MESSAGE_H

#include "Message.h"
#include "MathLib.h"

class FloatMessage : public Message {
public:
	FloatMessage(String message, float value);
	virtual ~FloatMessage();
	
	float value;
};

class IntegerMessage : public Message 
{
public:
	IntegerMessage(String message, int value);
	virtual ~IntegerMessage();
	int value;
};

/// Vector message that can hold various types of vectors.
class VectorMessage : public Message {
public:
	VectorMessage(String message, Vector2i vectorValue);
	VectorMessage(String message, Vector2f vectorValue);
	VectorMessage(String message, Vector3f vectorValue);
	VectorMessage(String message, Vector4f vectorValue);
	virtual ~VectorMessage();
	// Returns the values embedded within in the Vector4f format, converting as necessary.
	Vector4f GetVector4f();
	// Returns the values embedded within in the Vector4f format, converting as necessary.
	Vector3f GetVector3f();
	enum vectorTypes {
		VECTOR_2I,
		VECTOR_2F,
		VECTOR_3F,
		VECTOR_4F,
	};
	/// Specifies which kind of vector data is sent along in this message. See enum above.
	int vectorType;
	Vector2i vec2i;
	Vector2f vec2f;
	Vector3f vec3f;
	Vector4f vec4f;
};

#endif