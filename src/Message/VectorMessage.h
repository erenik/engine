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

/// Vector message that can hold various types of vectors.
class VectorMessage : public Message {
public:
	VectorMessage(String message);
	virtual ~VectorMessage();
	Vector2i vec2i;	
	Vector4f vec4f;
};

#endif